// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "CurlConnection.h"

#include "../ErrorHandling.h"

#include <curl/curl.h>

#include <cstring>

#define RETURN_IF_CURL_ERROR(curlCall, error)                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        auto __curlCode = (curlCall);                                                                                  \
        std::string __message = "Curl error: " + std::string(curl_easy_strerror(__curlCode));                          \
        RETURN_CODE_IF_LOG(error, __curlCode != CURLE_OK, m_handler, std::move(__message));                            \
    } while ((void)0, 0)

#define RETURN_IF_CURL_SETUP_ERROR(curlCall) RETURN_IF_CURL_ERROR(curlCall, ConnectionSetupFailed)
#define RETURN_IF_CURL_UNEXPECTED_ERROR(curlCall) RETURN_IF_CURL_ERROR(curlCall, ConnectionUnexpectedError)

// Setting a hard limit of 100k characters for the response to avoid rogue servers sending huge amounts of data
#define MAX_RESPONSE_CHARACTERS 100000

using namespace SFS;
using namespace SFS::details;

namespace
{
// Curl callback for writing data to a std::string. Must return the number of bytes written.
// This callback may be called multiple times for a single request, and will keep appending
// to userData until the request is complete. The data received is not null-terminated.
// For SFS, this data will likely be a JSON string.
size_t WriteCallback(char* contents, size_t sizeInBytes, size_t numElements, void* userData)
{
    auto readBufferPtr = static_cast<std::string*>(userData);
    if (readBufferPtr)
    {
        size_t totalSize = sizeInBytes * numElements;

        // Checking final response size to avoid unexpected amounts of data
        if ((readBufferPtr->length() + totalSize) > MAX_RESPONSE_CHARACTERS)
        {
            return CURL_WRITEFUNC_ERROR;
        }

        readBufferPtr->append(contents, totalSize);
        return totalSize;
    }
    return CURL_WRITEFUNC_ERROR;
}

enum class HttpHeader
{
    ContentType
};

std::string ToString(HttpHeader header)
{
    switch (header)
    {
    case HttpHeader::ContentType:
        return "Content-Type";
    }

    return "";
}

struct CurlHeaderList
{
  public:
    CurlHeaderList() = default;

    ~CurlHeaderList()
    {
        curl_slist_free_all(m_slist);
    }

    [[nodiscard]] Result Add(HttpHeader header, const std::string& value)
    {
        const std::string data = ToString(header) + ": " + value;
        const auto ret = curl_slist_append(m_slist, data.c_str());
        if (!ret)
        {
            return Result(Result::ConnectionSetupFailed, "Failed to add header to CurlHeaderList");
        }
        m_slist = ret;
        return Result::Success;
    }

    struct curl_slist* m_slist{nullptr};
};

struct CurlErrorBuffer
{
  public:
    CurlErrorBuffer(CURL* handle, const ReportingHandler& reportingHandler)
        : m_handle(handle)
        , m_reportingHandler(reportingHandler)
    {
        m_errorBuffer[0] = '\0';
        SetBuffer();
    }

    ~CurlErrorBuffer()
    {
        LOG_IF_FAILED(UnsetBuffer(), m_reportingHandler);
    }

    void SetBuffer()
    {
        THROW_CODE_IF_LOG(ConnectionSetupFailed,
                          curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, m_errorBuffer) != CURLE_OK,
                          m_reportingHandler,
                          "Failed to set up error buffer for curl");
    }

    Result UnsetBuffer()
    {
        return curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, nullptr) == CURLE_OK
                   ? Result::Success
                   : Result(Result::ConnectionSetupFailed, "Failed to unset curl error buffer");
    }

    char* Get()
    {
        return m_errorBuffer;
    }

  private:
    CURL* m_handle;
    const ReportingHandler& m_reportingHandler;

    char m_errorBuffer[CURL_ERROR_SIZE];
};

Result CurlCodeToResult(CURLcode curlCode, char* errorBuffer)
{
    Result::Code code;
    switch (curlCode)
    {
    case CURLE_OPERATION_TIMEDOUT:
        code = Result::HttpTimeout;
        break;
    default:
        code = Result::ConnectionUnexpectedError;
        break;
    }

    const bool isErrorStringRegistered = errorBuffer && errorBuffer[0] != '\0';
    std::string message = isErrorStringRegistered ? errorBuffer : "Curl error";

    return Result(code, std::move(message));
}

Result HttpCodeToResult(long httpCode)
{
    switch (httpCode)
    {
    case 200:
    {
        return Result::Success;
    }
    case 400:
    {
        return Result(Result::HttpBadRequest, "400 Bad Request");
    }
    case 404:
    {
        return Result(Result::HttpNotFound, "404 Not Found");
    }
    case 405:
    {
        return Result(Result::HttpBadRequest, "405 Method Not Allowed");
    }
    case 503:
    {
        return Result(Result::HttpServiceNotAvailable, "503 Service Unavailable");
    }
    default:
    {
        return Result(Result::HttpUnexpected, "Unexpected HTTP code " + std::to_string(httpCode));
    }
    }
}
} // namespace

Result CurlConnection::Make(const ReportingHandler& handler, std::unique_ptr<Connection>& out)
{
    auto tmp = std::unique_ptr<CurlConnection>(new CurlConnection(handler));
    RETURN_IF_FAILED_LOG(tmp->SetupCurl(), handler);
    out = std::move(tmp);

    return Result::Success;
}

CurlConnection::CurlConnection(const ReportingHandler& handler) : Connection(handler)
{
}

CurlConnection::~CurlConnection()
{
    if (m_handle)
    {
        curl_easy_cleanup(m_handle);
    }
}

Result CurlConnection::SetupCurl()
{
    m_handle = curl_easy_init();
    RETURN_CODE_IF_LOG(ConnectionSetupFailed, !m_handle, m_handler, "Failed to init curl connection");

    // Turning timeout signals off to avoid issues with threads
    // See https://curl.se/libcurl/c/threadsafe.html
    RETURN_CODE_IF_LOG(ConnectionSetupFailed,
                       curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L) != CURLE_OK,
                       m_handler,
                       "Failed to set up curl");

    // TODO #40: Allow passing user agent and MS-CV in the header
    // TODO #41: Pass AAD token in the header if it is available
    // TODO #42: Cert pinning with service

    return Result::Success;
}

Result CurlConnection::Get(const std::string& url, std::string& response)
{
    RETURN_CODE_IF_LOG(InvalidArg, url.empty(), m_handler, "url cannot be empty");

    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPGET, 1L));
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, nullptr));

    RETURN_IF_FAILED_LOG(CurlPerform(url, response), m_handler);

    return Result::Success;
}

Result CurlConnection::Post(const std::string& url, const std::string& data, std::string& response)
{
    RETURN_CODE_IF_LOG(InvalidArg, url.empty(), m_handler, "url cannot be empty");

    CurlHeaderList headerList;
    RETURN_IF_FAILED_LOG(headerList.Add(HttpHeader::ContentType, "application/json"), m_handler);

    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_POST, 1L));
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_COPYPOSTFIELDS, data.c_str()));
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, headerList.m_slist));

    RETURN_IF_FAILED_LOG(CurlPerform(url, response), m_handler);

    return Result::Success;
}

Result CurlConnection::CurlPerform(const std::string& url, std::string& response)
{
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str()));

    // Setting up error buffer where error messages get written - this gets unset in the destructor
    CurlErrorBuffer errorBuffer(m_handle, m_handler);

    std::string readBuffer;
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, WriteCallback));
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &readBuffer));

    auto result = curl_easy_perform(m_handle);
    if (result != CURLE_OK)
    {
        return CurlCodeToResult(result, errorBuffer.Get());
    }

    response = std::move(readBuffer);

    // TODO #43: perform retry logic according to response errors
    // The retry logic should also be opt-out-able by the user

    long httpCode = 0;
    RETURN_IF_CURL_UNEXPECTED_ERROR(curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &httpCode));
    return HttpCodeToResult(httpCode);
}
