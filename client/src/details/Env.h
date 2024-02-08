// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"

#include <memory>
#include <optional>
#include <string>

namespace SFS::details::env
{
/**
 * @brief Get the value of an environment variable.
 * @details std::nullopt is returned if the environment variable is not set or in case of failure.
 * The returned string is encoded in ASCII in Windows, not UTF-8.
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
    [[nodiscard]] static Result Make(std::string varName, const std::string& value, std::unique_ptr<ScopedEnv>& out);
    ~ScopedEnv();

  private:
    ScopedEnv(std::string varName);

    std::string m_varName;
    std::optional<std::string> m_oldValue;
};
} // namespace SFS::details::env
