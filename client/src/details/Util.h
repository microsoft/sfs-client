// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string_view>

namespace SFS::details::util
{
bool AreEqualI(std::string_view a, std::string_view b);
bool AreNotEqualI(std::string_view a, std::string_view b);

std::string GetLatestVersionUrl(const std::string& baseUrl,
                                const std::string& instanceId,
                                const std::string& nameSpace);

std::string GetSpecificVersionUrl(const std::string& baseUrl,
                                  const std::string& instanceId,
                                  const std::string& nameSpace,
                                  const std::string& productName,
                                  const std::string& version);

std::string GetDownloadInfoUrl(const std::string& baseUrl,
                               const std::string& instanceId,
                               const std::string& nameSpace,
                               const std::string& productName,
                               const std::string& version);
} // namespace SFS::details::util
