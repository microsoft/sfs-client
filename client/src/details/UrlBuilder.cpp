// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "UrlBuilder.h"

#include "ErrorHandling.h"
#include "ReportingHandler.h"

#include <curl/curl.h>

#define THROW_IF_CURLU_ERROR(curlCall, error)                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        auto __curlUrlCode = (curlCall);                                                                               \
        THROW_CODE_IF_NOT_LOG(error, __curlUrlCode == CURLUE_OK, m_handler, GetCurlUrlStrError(__curlUrlCode));        \
    } while ((void)0, 0)

#define THROW_IF_CURLU_SETUP_ERROR(curlCall) THROW_IF_CURLU_ERROR(curlCall, ConnectionUrlSetupFailed)

using namespace SFS::details;

namespace
{
struct CurlString
{
  public:
    CurlString() = default;

    ~CurlString()
    {
        Release();
    }

    CurlString(const CurlString&) = delete;
    CurlString& operator=(const CurlString&) = delete;

    char* Get() const
    {
        return m_data;
    }

    char** ReleaseAndGetAddressOf()
    {
        Release();
        return &m_data;
    }

  private:
    void Release()
    {
        if (m_data)
        {
            curl_free(m_data);
        }
    }

    char* m_data = nullptr;
};

std::string GetCurlUrlStrError(CURLUcode code)
{
    return "Curl URL error: " + std::string(curl_url_strerror(code));
}
} // namespace

UrlBuilder::UrlBuilder(const ReportingHandler& handler) : m_handler(handler)
{
    m_handle = curl_url();
}

UrlBuilder::~UrlBuilder()
{
    curl_url_cleanup(m_handle);
}

std::string UrlBuilder::GetUrl() const
{
    CurlString url;
    THROW_IF_CURLU_SETUP_ERROR(curl_url_get(m_handle, CURLUPART_URL, url.ReleaseAndGetAddressOf(), 0 /*flags*/));
    return url.Get();
}

void UrlBuilder::SetScheme(Scheme scheme)
{
    switch (scheme)
    {
    case Scheme::Https:
        THROW_IF_CURLU_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_SCHEME, "https", 0 /*flags*/));
        break;
    }
}

void UrlBuilder::SetHost(const std::string& host)
{
    THROW_IF_CURLU_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_HOST, host.c_str(), 0 /*flags*/));
}

void UrlBuilder::SetPath(const std::string& path, bool encode)
{
    unsigned flags = 0;
    if (encode)
    {
        flags |= CURLU_URLENCODE;
    }
    THROW_IF_CURLU_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_PATH, path.c_str(), flags));
}

void UrlBuilder::SetQuery(const std::string& query, bool append)
{
    unsigned flags = 0;
    if (append)
    {
        flags |= CURLU_APPENDQUERY;
    }
    THROW_IF_CURLU_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_QUERY, query.c_str(), flags));
}

void UrlBuilder::SetUrl(const std::string& url)
{
    THROW_IF_CURLU_SETUP_ERROR(curl_url_set(m_handle, CURLUPART_URL, url.c_str(), 0 /*flags*/));
}
