// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "CurlConnection.h"

#include "../ErrorHandling.h"

#include <curl/curl.h>

#include <cstring>

#define RETURN_IF_CURL_ERROR(curlCall)                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        auto __curlCode = (curlCall);                                                                                  \
        std::string __message = "Curl error: " + std::string(curl_easy_strerror(__curlCode));                          \
        RETURN_CODE_IF_LOG(E_HttpUnexpected, __curlCode != CURLE_OK, m_handler, std::move(__message));                 \
    } while ((void)0, 0)

using namespace SFS;
using namespace SFS::details;

namespace
{
// Curl callback for writing data to a std::string. Must return the number of bytes written.
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

struct CurlSList
{
  public:
    CurlSList() = default;
    ~CurlSList();

    void Append(const char* data);

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
        code = Result::E_HttpUnexpected;
        break;
    }

    const bool isErrorStringRegistered = strlen(errorBuffer) > 0;
    std::string message = isErrorStringRegistered ? std::string(errorBuffer) : "Curl error";

    return Result(code, std::move(message));
}

Result HttpCodeToResult(long httpCode)
{
    if (httpCode == 400)
    {
        return Result(Result::E_HttpBadRequest, "400 Bad Request");
    }
    else if (httpCode == 404)
    {
        return Result(Result::E_HttpNotFound, "404 Not Found");
    }
    else if (httpCode == 405)
    {
        return Result(Result::E_HttpBadRequest, "405 Method Not Allowed");
    }
    else if (httpCode == 503)
    {
        return Result(Result::E_HttpServiceNotAvailable, "503 Service Unavailable");
    }
    else if (httpCode != 200)
    {
        return Result(Result::E_HttpUnexpected, "Unexpected HTTP code");
    }

    return Result::S_Ok;
}
} // namespace

CurlConnection::CurlConnection(const ReportingHandler& handler) : Connection(handler)
{
    m_handle = curl_easy_init();
    THROW_CODE_IF_LOG(E_HttpUnexpected, !m_handle, m_handler, "Failed to init curl connection");

    // Turning timeout signals off to avoid issues with threads
    // See https://curl.se/libcurl/c/threadsafe.html
    THROW_CODE_IF_LOG(E_HttpUnexpected,
                      curl_easy_setopt(m_handle, CURLOPT_NOSIGNAL, 1L) != CURLE_OK,
                      m_handler,
                      "Failed to set up curl");

    // Setting up error buffer where error messages get written
    THROW_CODE_IF_LOG(E_HttpUnexpected,
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

    RETURN_IF_CURL_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPGET, 1L));
    RETURN_IF_CURL_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, nullptr));

    RETURN_IF_FAILED(CurlPerform(url, response));

    return Result::S_Ok;
}

Result CurlConnection::Post(std::string_view url, std::string_view data, std::string& response)
{
    RETURN_CODE_IF_LOG(E_InvalidArg, url.empty(), m_handler, "url cannot be empty");

    CurlSList slist;
    slist.Append("Content-Type: application/json");

    RETURN_IF_CURL_ERROR(curl_easy_setopt(m_handle, CURLOPT_POST, 1L));
    RETURN_IF_CURL_ERROR(curl_easy_setopt(m_handle, CURLOPT_COPYPOSTFIELDS, data.empty() ? "" : data.data()));
    RETURN_IF_CURL_ERROR(curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, slist.m_slist));

    RETURN_IF_FAILED(CurlPerform(url, response));

    return Result::S_Ok;
}

Result CurlConnection::CurlPerform(std::string_view url, std::string& response)
{
    RETURN_IF_CURL_ERROR(curl_easy_setopt(m_handle, CURLOPT_URL, url.data()));

    std::string readBuffer;
    RETURN_IF_CURL_ERROR(curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, WriteCallback));
    RETURN_IF_CURL_ERROR(curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, &readBuffer));

    auto result = curl_easy_perform(m_handle);
    if (result != CURLE_OK)
    {
        return CurlCodeToResult(result, m_errorBuffer);
    }

    response = std::move(readBuffer);

    // TODO: perform retry logic according to response errors
    // The retry logic should also be opt-out-able by the user

    long httpCode = 0;
    RETURN_IF_CURL_ERROR(curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &httpCode));
    return HttpCodeToResult(httpCode);
}

void CurlSList::Append(const char* data)
{
    m_slist = curl_slist_append(m_slist, data);
}

CurlSList::~CurlSList()
{
    curl_slist_free_all(m_slist);
}
