// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "UrlBuilder.h"

#include <memory>

namespace SFS::details
{
class SFSUrlBuilder : private UrlBuilder
{
  public:
    static std::unique_ptr<SFSUrlBuilder> CreateFromAccountId(const std::string& accountId,
                                                              std::string instanceId,
                                                              std::string nameSpace,
                                                              const ReportingHandler& handler);

    static std::unique_ptr<SFSUrlBuilder> CreateFromCustomUrl(const std::string& customUrl,
                                                              std::string instanceId,
                                                              std::string nameSpace,
                                                              const ReportingHandler& handler);

    std::string GetLatestVersionUrl(const std::string& product);
    std::string GetLatestVersionBatchUrl();
    std::string GetSpecificVersionUrl(const std::string& product, const std::string& version);
    std::string GetDownloadInfoUrl(const std::string& product, const std::string& version);

    using UrlBuilder::GetUrl;

  private:
    SFSUrlBuilder(std::string&& instanceId, std::string&& nameSpace, const ReportingHandler& handler);

    std::string GetNamesUrlPath() const;
    std::string GetVersionsUrlPath(const std::string& product) const;

    std::string m_instanceId;
    std::string m_nameSpace;
};
} // namespace SFS::details
