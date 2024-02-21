// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

namespace SFS::details
{
class SFSUrlComponents
{
  public:
    static std::string GetLatestVersionUrl(const std::string& baseUrl,
                                           const std::string& instanceId,
                                           const std::string& nameSpace,
                                           const std::string& productName);

    static std::string GetLatestVersionBatchUrl(const std::string& baseUrl,
                                                const std::string& instanceId,
                                                const std::string& nameSpace);

    static std::string GetSpecificVersionUrl(const std::string& baseUrl,
                                             const std::string& instanceId,
                                             const std::string& nameSpace,
                                             const std::string& productName,
                                             const std::string& version);

    static std::string GetDownloadInfoUrl(const std::string& baseUrl,
                                          const std::string& instanceId,
                                          const std::string& nameSpace,
                                          const std::string& productName,
                                          const std::string& version);
};
} // namespace SFS::details
