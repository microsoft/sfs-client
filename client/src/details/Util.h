// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace SFS::details
{
class ReportingHandler;
}

namespace SFS::details::util
{
bool AreEqualI(std::string_view a, std::string_view b);
bool AreNotEqualI(std::string_view a, std::string_view b);

bool AreTestOverridesAllowed(const ReportingHandler& handler);
std::optional<std::string> GetEnv(const char* varName);
} // namespace SFS::details::util
