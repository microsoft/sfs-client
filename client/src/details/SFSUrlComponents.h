// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

namespace SFS::details
{
class ReportingHandler;

class SFSUrlComponents
{
  public:
    static std::string GetLatestVersionUrl(const std::string& baseUrl,
                                           const std::string& instanceId,
                                           const std::string& nameSpace,
                                           const std::string& product,
                                           const ReportingHandler& handler);

    static std::string GetLatestVersionBatchUrl(const std::string& baseUrl,
                                                const std::string& instanceId,
                                                const std::string& nameSpace,
                                                const ReportingHandler& handler);

    static std::string GetSpecificVersionUrl(const std::string& baseUrl,
                                             const std::string& instanceId,
                                             const std::string& nameSpace,
                                             const std::string& product,
                                             const std::string& version,
                                             const ReportingHandler& handler);

    static std::string GetDownloadInfoUrl(const std::string& baseUrl,
                                          const std::string& instanceId,
                                          const std::string& nameSpace,
                                          const std::string& product,
                                          const std::string& version,
                                          const ReportingHandler& handler);

    /**
     * @brief Escape the string @param str to be used in a URL
     * @return The escaped string
     */
    static std::string UrlEscape(const std::string& str, const ReportingHandler& handler);
};
} // namespace SFS::details
