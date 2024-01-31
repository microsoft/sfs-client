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

/**
 * @brief Check if test overrides are allowed and logs if so.
 * @details Test overrides are allowed if the SFS_ENABLE_TEST_OVERRIDES macro is defined.
 */
bool AreTestOverridesAllowed(const ReportingHandler& handler);

/**
 * @brief Get the value of an environment variable.
 * @details std::nullopt is returned if the environment variable is not set or in case of failure.
 */
std::optional<std::string> GetEnv(const char* varName);
} // namespace SFS::details::util
