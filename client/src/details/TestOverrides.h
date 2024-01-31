// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>

namespace SFS::details
{
class ReportingHandler;
}

namespace SFS::details::util
{
/**
 * @brief Check if test overrides are allowed and logs if so.
 * @details Test overrides are allowed if the SFS_ENABLE_TEST_OVERRIDES macro is defined.
 */
bool AreTestOverridesAllowed(const ReportingHandler& handler);

/**
 * @brief Get the value of an environment variable.
 * @details std::nullopt is returned if the environment variable is not set or in case of failure.
 * The returned string may be different in Win32 due to the encoding of the environment variables.
 */
std::optional<std::string> GetEnv(const std::string& varName);

/**
 * @brief Set the value of an environment variable.
 * @return false in case of failure.
 */
bool SetEnv(const std::string& varName, const std::string& value);

/**
 * @brief Unset the value of an environment variable.
 * @return false in case of failure. If the environment variable didn't exist, it still returns true.
 */
bool UnsetEnv(const std::string& varName);

class ScopedEnv
{
  public:
    ScopedEnv(std::string varName, const std::string& value);
    ~ScopedEnv();

  private:
    std::string m_varName;
    std::optional<std::string> m_oldValue;
};
} // namespace SFS::details::util
