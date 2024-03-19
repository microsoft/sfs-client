// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "UrlBuilder.h"

#include "ErrorHandling.h"
#include "ReportingHandler.h"

#include <curl/curl.h>

#include <memory>

#define THROW_IF_CURL_URL_ERROR(curlCall, error)                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        auto __curlUrlCode = (curlCall);                                                                               \
        THROW_CODE_IF_NOT_LOG(error, __curlUrlCode == CURLUE_OK, m_handler, GetCurlUrlStrError(__curlUrlCode));        \
    } while ((void)0, 0)

#define THROW_IF_CURL_URL_SETUP_ERROR(curlCall) THROW_IF_CURL_URL_ERROR(curlCall, ConnectionUrlSetupFailed)

using namespace SFS::details;

namespace
{

struct CurlEscapedUrlString
{
  public:
    CurlEscapedUrlString(const std::string& str, const ReportingHandler& handler)
    {
        m_escapedString = curl_easy_escape(nullptr /*ignored*/, str.c_str(), static_cast<int>(str.length()));
        THROW_CODE_IF_NOT_LOG(ConnectionUrlSetupFailed, m_escapedString, handler, "Failed to escape URL string");
    }

    ~CurlEscapedUrlString()
    {
        curl_free(m_escapedString);
    }

    CurlEscapedUrlString(const CurlEscapedUrlString&) = delete;
    CurlEscapedUrlString& operator=(const CurlEscapedUrlString&) = delete;

    char* Get()
    {
        return m_escapedString;
    }

  private:
    char* m_escapedString = nullptr;
};

struct CurlCharDeleter
{
    void operator()(char* val)
    {
        if (val)
        {
            curl_free(val);
        }
    }
};

using CurlCharPtr = std::unique_ptr<char, CurlCharDeleter>;

std::string GetCurlUrlStrError(CURLUcode code)
{
    return "Curl URL error: " + std::string(curl_url_strerror(code));
}
} // namespace

UrlBuilder::UrlBuilder(const ReportingHandler& handler) : m_handler(handler)
{
    m_handle = curl_url();
    THROW_CODE_IF_NOT_LOG(ConnectionUrlSetupFailed, m_handle, m_handler, "Curl URL error: Failed to create URL");
}

UrlBuilder::UrlBuilder(const std::string& url, const ReportingHandler& handler) : UrlBuilder(handler)
{
    SetUrl(url);
}

UrlBuilder::~UrlBuilder()
{
    curl_url_cleanup(m_handle);
}

std::string UrlBuilder::GetUrl() const
{
    CurlCharPtr url;
    char* urlPtr = url.get();
    THROW_IF_CURL_URL_SETUP_ERROR(curl_url_get(m_handle, CURLUPART_URL, &urlPtr, 0 /*flags*/));
    return urlPtr;
}

void UrlBuilder::SetScheme(Scheme scheme)
{
    switch (scheme)
    {
    case Scheme::Https:
        THROW_IF_CURL_URL_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_SCHEME, "https", 0 /*flags*/));
        break;
    }
}

void UrlBuilder::SetHost(const std::string& host)
{
    THROW_IF_CURL_URL_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_HOST, host.c_str(), 0 /*flags*/));
}

void UrlBuilder::SetPath(const std::string& path, bool encode)
{
    unsigned flags = 0;
    if (encode)
    {
        flags |= CURLU_URLENCODE;
    }
    THROW_IF_CURL_URL_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_PATH, path.c_str(), flags));
}

void UrlBuilder::SetQuery(const std::string& query)
{
    THROW_IF_CURL_URL_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_QUERY, query.c_str(), 0 /*flags*/));
}

void UrlBuilder::AppendQuery(const std::string& query)
{
    THROW_IF_CURL_URL_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_QUERY, query.c_str(), CURLU_APPENDQUERY));
}

void UrlBuilder::SetUrl(const std::string& url)
{
    THROW_IF_CURL_URL_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_URL, url.c_str(), 0 /*flags*/));
}

std::string UrlBuilder::EscapeString(const std::string& str) const
{
    CurlEscapedUrlString escapedStr(str, m_handler);
    return escapedStr.Get();
}
