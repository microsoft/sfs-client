// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSUrlBuilder.h"

using namespace SFS::details;

constexpr const char* c_apiVersion = "v2";
constexpr const char* c_apiDomain = "api.cdp.microsoft.com";

std::unique_ptr<SFSUrlBuilder> SFSUrlBuilder::CreateFromAccountId(const std::string& accountId,
                                                                  std::string instanceId,
                                                                  std::string nameSpace,
                                                                  const ReportingHandler& handler)
{
    auto builder =
        std::unique_ptr<SFSUrlBuilder>(new SFSUrlBuilder(std::move(instanceId), std::move(nameSpace), handler));
    builder->SetScheme(Scheme::Https);
    builder->SetHost(accountId + "." + std::string(c_apiDomain));
    return builder;
}

std::unique_ptr<SFSUrlBuilder> SFSUrlBuilder::CreateFromCustomUrl(const std::string& customUrl,
                                                                  std::string instanceId,
                                                                  std::string nameSpace,
                                                                  const ReportingHandler& handler)
{
    auto builder =
        std::unique_ptr<SFSUrlBuilder>(new SFSUrlBuilder(std::move(instanceId), std::move(nameSpace), handler));
    builder->SetUrl(customUrl);
    return builder;
}

SFSUrlBuilder::SFSUrlBuilder(std::string&& instanceId, std::string&& nameSpace, const ReportingHandler& handler)
    : UrlBuilder(handler)
    , m_instanceId(std::move(instanceId))
    , m_nameSpace(std::move(nameSpace))
{
}

std::string SFSUrlBuilder::GetLatestVersionUrl(const std::string& product)
{
    SetPath(GetVersionsUrlPath(product) + "latest");
    SetQuery("action=select");
    return GetUrl();
}

std::string SFSUrlBuilder::GetLatestVersionBatchUrl()
{
    SetPath(GetNamesUrlPath());
    SetQuery("action=BatchUpdates");
    return GetUrl();
}

std::string SFSUrlBuilder::GetSpecificVersionUrl(const std::string& product, const std::string& version)
{
    SetPath(GetVersionsUrlPath(product) + EscapeString(version));
    SetQuery("");
    return GetUrl();
}

std::string SFSUrlBuilder::GetDownloadInfoUrl(const std::string& product, const std::string& version)
{
    SetPath(GetVersionsUrlPath(product) + EscapeString(version) + "/files");
    SetQuery("action=GenerateDownloadInfo");
    return GetUrl();
}

std::string SFSUrlBuilder::GetNamesUrlPath() const
{
    return "api/" + std::string(c_apiVersion) + "/contents/" + EscapeString(m_instanceId) + "/namespaces/" +
           EscapeString(m_nameSpace) + "/names";
}

std::string SFSUrlBuilder::GetVersionsUrlPath(const std::string& product) const
{
    return GetNamesUrlPath() + "/" + EscapeString(product) + "/versions/";
}
