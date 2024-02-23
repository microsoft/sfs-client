// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace SFS
{
using TargetingAttributes = std::unordered_map<std::string, std::string>;

struct ProductRequest
{
    /// @brief The name or GUID that uniquely represents the product in the service (required)
    std::string product;

    /// @brief Key-value pair to filter the data retrieved from the service. Known from publishing (optional)
    TargetingAttributes attributes;
};

/// @brief Configurations to perform a request to the SFS service
struct RequestParams
{
    /// @brief List of products to be retrieved from the server (required)
    /// @note At the moment only a single product request is supported. Using a vector for future implementation of
    /// batch requests
    std::vector<ProductRequest> productRequests;
};
} // namespace SFS
