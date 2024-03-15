// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSUrlComponents.h"

#include "ErrorHandling.h"
#include "ReportingHandler.h"

#include <curl/curl.h>

using namespace SFS::details;

namespace
{
struct CurlEscapedString
{
  public:
    CurlEscapedString(const std::string& str, const ReportingHandler& handler)
    {
        m_escapedString = curl_easy_escape(nullptr /*ignored*/, str.c_str(), static_cast<int>(str.length()));
        THROW_CODE_IF_LOG(Unexpected,
                          m_escapedString == nullptr,
                          handler,
                          "Failed to escape string using curl_easy_escape");
    }

    ~CurlEscapedString()
    {
        curl_free(m_escapedString);
    }

    CurlEscapedString(const CurlEscapedString&) = delete;
    CurlEscapedString& operator=(const CurlEscapedString&) = delete;

    char* Get()
    {
        return m_escapedString;
    }

  private:
    char* m_escapedString = nullptr;
};

std::string GetNamesUrlComponent(const std::string& baseUrl,
                                 const std::string& instanceId,
                                 const std::string& nameSpace,
                                 const ReportingHandler& handler)
{
    // Currently using same v2 API for all URLs of the Client
    return baseUrl + "/api/v2/contents/" + SFSUrlComponents::UrlEscape(instanceId, handler) + "/namespaces/" +
           SFSUrlComponents::UrlEscape(nameSpace, handler) + "/names";
}

std::string GetVersionsUrlComponent(const std::string& baseUrl,
                                    const std::string& instanceId,
                                    const std::string& nameSpace,
                                    const std::string& product,
                                    const ReportingHandler& handler)
{
    return GetNamesUrlComponent(baseUrl, instanceId, nameSpace, handler) + "/" +
           SFSUrlComponents::UrlEscape(product, handler) + "/versions/";
}
} // namespace

std::string SFSUrlComponents::GetLatestVersionUrl(const std::string& baseUrl,
                                                  const std::string& instanceId,
                                                  const std::string& nameSpace,
                                                  const std::string& product,
                                                  const ReportingHandler& handler)
{
    return GetVersionsUrlComponent(baseUrl, instanceId, nameSpace, product, handler) + "latest?action=select";
}

std::string SFSUrlComponents::GetLatestVersionBatchUrl(const std::string& baseUrl,
                                                       const std::string& instanceId,
                                                       const std::string& nameSpace,
                                                       const ReportingHandler& handler)
{
    return GetNamesUrlComponent(baseUrl, instanceId, nameSpace, handler) + "?action=BatchUpdates";
}

std::string SFSUrlComponents::GetSpecificVersionUrl(const std::string& baseUrl,
                                                    const std::string& instanceId,
                                                    const std::string& nameSpace,
                                                    const std::string& product,
                                                    const std::string& version,
                                                    const ReportingHandler& handler)
{
    return GetVersionsUrlComponent(baseUrl, instanceId, nameSpace, product, handler) +
           SFSUrlComponents::UrlEscape(version, handler);
}

std::string SFSUrlComponents::GetDownloadInfoUrl(const std::string& baseUrl,
                                                 const std::string& instanceId,
                                                 const std::string& nameSpace,
                                                 const std::string& product,
                                                 const std::string& version,
                                                 const ReportingHandler& handler)
{
    return GetVersionsUrlComponent(baseUrl, instanceId, nameSpace, product, handler) +
           SFSUrlComponents::UrlEscape(version, handler) + "/files?action=GenerateDownloadInfo";
}

std::string SFSUrlComponents::UrlEscape(const std::string& str, const ReportingHandler& handler)
{
    return CurlEscapedString(str, handler).Get();
}
