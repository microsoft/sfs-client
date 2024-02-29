// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>

namespace SFS::details
{
struct ConnectionConfig
{
    /// @brief Expected number of retries for a web request after a failed attempt
    unsigned maxRetries{3};

    /// @brief The correlation vector to use for requests
    std::optional<std::string> baseCV;
};
} // namespace SFS::details
