// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string_view>
#include <string>
#include <optional>

namespace SFS::details::util
{
bool AreEqualI(std::string_view a, std::string_view b);
bool AreNotEqualI(std::string_view a, std::string_view b);
std::optional<std::string> GetEnv(const char* varName);
} // namespace SFS::details::util
