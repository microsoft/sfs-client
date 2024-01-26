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

#define RETURN_IF_CURL_SETUP_ERROR(curlCall) RETURN_IF_CURL_ERROR(curlCall, E_ConnectionSetupFailed)
#define RETURN_IF_CURL_UNEXPECTED_ERROR(curlCall) RETURN_IF_CURL_ERROR(curlCall, E_ConnectionUnexpectedError)

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
        auto totalSize = sizeInBytes * numElements;
        readBufferPtr->append(contents, totalSize);
        return totalSize;
    }
    return 0;
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
    ~CurlHeaderList();

    void Add(HttpHeader header, const std::string& value);

    struct curl_slist* m_slist{nullptr};
};

Result CurlCodeToResult(CURLcode curlCode, char* errorBuffer)
{
    Result::Code code;
    switch (curlCode)
    {
    case CURLE_OPERATION_TIMEDOUT:
        code = Result::E_HttpTimeout;
        break;
    default:
        code = Result::E_ConnectionUnexpectedError;
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
        return Result::S_Ok;
    }
    case 400:
    {
        return Result(Result::E_HttpBadRequest, "400 Bad Request");
    }
    case 404:
    {
        return Result(Result::E_HttpNotFound, "404 Not Found");
    }
    case 405:
    {
        return Result(Result::E_HttpBadRequest, "405 Method Not Allowed");
    }
    case 503:
    {
        return Result(Result::E_HttpServiceNotAvailable, "503 Service Unavailable");
    }
    default:
    {
        return Result(Result::E_HttpUnexpected, "Unexpected HTTP code " + std::to_string(httpCode));
    }
    }
}
} // namespace

CurlConnection::CurlConnection(const ReportingHandler& handler) : Connection(handler)
{
    m_handle = curl_easy_init();
    THROW_CODE_IF_LOG(E_ConnectionSetupFailed, !m_handle, m_handler, "Failed to init curl connection");

    // Turning timeout signals off to avoid issues with threads
    // See https://curl.se/libcurl/c/threadsafe.html
    THROW_CODE_IF_LOG(E_ConnectionSetupFailed,
                      curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L) != CURLE_OK,
                      m_handler,
                      "Failed to set up curl");

    // Setting up error buffer where error messages get written
    THROW_CODE_IF_LOG(E_ConnectionSetupFailed,
                      curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, m_errorBuffer) != CURLE_OK,
                      m_handler,
                      "Failed to set up curl");

    // TODO: Pass AAD token in the header if it is available
    // TODO: Allow passing user agent and MS-CV in the header
    // TODO: Cert pinning with service
}

CurlConnection::~CurlConnection()
{
    if (m_handle)
    {
        curl_easy_cleanup(m_handle);
    }
}

Result CurlConnection::Get(std::string_view url, std::string& response)
{
    RETURN_CODE_IF_LOG(E_InvalidArg, url.empty(), m_handler, "url cannot be empty");

    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPGET, 1L));
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, nullptr));

    RETURN_IF_FAILED_LOG(CurlPerform(url, response), m_handler);

    return Result::S_Ok;
}

Result CurlConnection::Post(std::string_view url, std::string_view data, std::string& response)
{
    RETURN_CODE_IF_LOG(E_InvalidArg, url.empty(), m_handler, "url cannot be empty");

    CurlHeaderList headerList;
    headerList.Add(HttpHeader::ContentType, "application/json");

    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_POST, 1L));
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_COPYPOSTFIELDS, data.empty() ? "" : data.data()));
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, headerList.m_slist));

    RETURN_IF_FAILED_LOG(CurlPerform(url, response), m_handler);

    return Result::S_Ok;
}

Result CurlConnection::CurlPerform(std::string_view url, std::string& response)
{
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_URL, url.data()));

    std::string readBuffer;
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, WriteCallback));
    RETURN_IF_CURL_SETUP_ERROR(curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &readBuffer));

    auto result = curl_easy_perform(m_handle);
    if (result != CURLE_OK)
    {
        return CurlCodeToResult(result, m_errorBuffer);
    }

    response = std::move(readBuffer);

    // TODO: perform retry logic according to response errors
    // The retry logic should also be opt-out-able by the user

    long httpCode = 0;
    RETURN_IF_CURL_UNEXPECTED_ERROR(curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &httpCode));
    return HttpCodeToResult(httpCode);
}

void CurlHeaderList::Add(HttpHeader header, const std::string& value)
{
    const std::string data = ToString(header) + ": " + value;
    m_slist = curl_slist_append(m_slist, data.c_str());
}

CurlHeaderList::~CurlHeaderList()
{
    curl_slist_free_all(m_slist);
}
