// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Util.h"

#include <string>

using namespace SFS::details;

bool util::AreEqualI(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
    {
        return false;
    }

    for (auto itA = a.begin(), itB = b.begin(); itA != a.end() && itB != b.end(); ++itA, ++itB)
    {
        if (std::tolower(*itA) != std::tolower(*itB))
        {
            return false;
        }
    }
    return true;
}

bool util::AreNotEqualI(std::string_view a, std::string_view b)
{
    return !AreEqualI(a, b);
}

std::string util::GetLatestVersionUrl(const std::string& baseUrl,
                                      const std::string& instanceId,
                                      const std::string& nameSpace)
{
    return baseUrl + "/api/v2/contents/" + instanceId + "/namespaces/" + nameSpace + "/names?action=BatchUpdates";
}

std::string util::GetSpecificVersionUrl(const std::string& baseUrl,
                                        const std::string& instanceId,
                                        const std::string& nameSpace,
                                        const std::string& productName,
                                        const std::string& version)
{
    return baseUrl + "/api/v1/contents/" + instanceId + "/namespaces/" + nameSpace + "/names/" + productName +
           "/versions/" + version;
}

std::string util::GetDownloadInfoUrl(const std::string& baseUrl,
                                     const std::string& instanceId,
                                     const std::string& nameSpace,
                                     const std::string& productName,
                                     const std::string& version)
{
    return baseUrl + "/api/v1/contents/" + instanceId + "/namespaces/" + nameSpace + "/names/" + productName +
           "/versions/" + version + "/files?action=GenerateDownloadInfo";
}
