// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSUrlComponents.h"

using namespace SFS::details;

namespace
{
std::string GetNamesUrlComponent(const std::string& baseUrl,
                                 const std::string& instanceId,
                                 const std::string& nameSpace)
{
    // Currently using same v2 API for all URLs of the Client
    return baseUrl + "/api/v2/contents/" + instanceId + "/namespaces/" + nameSpace + "/names";
}

std::string GetVersionsUrlComponent(const std::string& baseUrl,
                                    const std::string& instanceId,
                                    const std::string& nameSpace,
                                    const std::string& product)
{
    return GetNamesUrlComponent(baseUrl, instanceId, nameSpace) + "/" + product + "/versions/";
}
} // namespace

std::string SFSUrlComponents::GetLatestVersionUrl(const std::string& baseUrl,
                                                  const std::string& instanceId,
                                                  const std::string& nameSpace,
                                                  const std::string& product)
{
    return GetVersionsUrlComponent(baseUrl, instanceId, nameSpace, product) + "latest?action=select";
}

std::string SFSUrlComponents::GetLatestVersionBatchUrl(const std::string& baseUrl,
                                                       const std::string& instanceId,
                                                       const std::string& nameSpace)
{
    return GetNamesUrlComponent(baseUrl, instanceId, nameSpace) + "?action=BatchUpdates";
}

std::string SFSUrlComponents::GetSpecificVersionUrl(const std::string& baseUrl,
                                                    const std::string& instanceId,
                                                    const std::string& nameSpace,
                                                    const std::string& product,
                                                    const std::string& version)
{
    return GetVersionsUrlComponent(baseUrl, instanceId, nameSpace, product) + version;
}

std::string SFSUrlComponents::GetDownloadInfoUrl(const std::string& baseUrl,
                                                 const std::string& instanceId,
                                                 const std::string& nameSpace,
                                                 const std::string& product,
                                                 const std::string& version)
{
    return GetVersionsUrlComponent(baseUrl, instanceId, nameSpace, product) + version +
           "/files?action=GenerateDownloadInfo";
}
