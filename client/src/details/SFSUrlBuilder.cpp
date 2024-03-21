// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSUrlBuilder.h"

using namespace SFS::details;

constexpr const char* c_apiVersion = "v2";
constexpr const char* c_apiDomain = "api.cdp.microsoft.com";

SFSUrlBuilder::SFSUrlBuilder(const std::string& accountId,
                             std::string instanceId,
                             std::string nameSpace,
                             const ReportingHandler& handler)
    : UrlBuilder(handler)
    , m_instanceId(std::move(instanceId))
    , m_nameSpace(std::move(nameSpace))
{
    SetScheme(Scheme::Https);
    SetHost(accountId + "." + std::string(c_apiDomain));
}

SFSUrlBuilder::SFSUrlBuilder(const SFSCustomUrl& customUrl,
                             std::string instanceId,
                             std::string nameSpace,
                             const ReportingHandler& handler)
    : UrlBuilder(handler)
    , m_instanceId(std::move(instanceId))
    , m_nameSpace(std::move(nameSpace))
{
    SetUrl(customUrl.url);
}

std::string SFSUrlBuilder::GetLatestVersionUrl(const std::string& product)
{
    SetVersionsUrlPath(product);
    AppendPath("latest");
    SetQuery("action=select");
    return GetUrl();
}

std::string SFSUrlBuilder::GetLatestVersionBatchUrl()
{
    SetNamesUrlPath();
    SetQuery("action=BatchUpdates");
    return GetUrl();
}

std::string SFSUrlBuilder::GetSpecificVersionUrl(const std::string& product, const std::string& version)
{
    SetVersionsUrlPath(product);
    AppendPath(version, true /*encode*/);
    return GetUrl();
}

std::string SFSUrlBuilder::GetDownloadInfoUrl(const std::string& product, const std::string& version)
{
    SetVersionsUrlPath(product);
    AppendPath(version, true /*encode*/);
    AppendPath("files");
    SetQuery("action=GenerateDownloadInfo");
    return GetUrl();
}

SFSUrlBuilder& SFSUrlBuilder::ResetPathAndQuery()
{
    SetPath("").SetQuery("");
    return *this;
}

SFSUrlBuilder& SFSUrlBuilder::SetNamesUrlPath()
{
    ResetPathAndQuery();
    AppendPath("api").AppendPath(c_apiVersion);
    AppendPath("contents").AppendPath(m_instanceId, true /*encode*/);
    AppendPath("namespaces").AppendPath(m_nameSpace, true /*encode*/);
    AppendPath("names");
    return *this;
}

SFSUrlBuilder& SFSUrlBuilder::SetVersionsUrlPath(const std::string& product)
{
    SetNamesUrlPath();
    AppendPath(product, true /*encode*/);
    AppendPath("versions");
    return *this;
}
