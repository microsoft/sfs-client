// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Content.h"
#include "DeliveryOptimizationData.h"

#include <nlohmann/json_fwd.hpp>

#include <memory>

namespace SFS::details
{
class ReportingHandler;

namespace contentutil
{
//
// JSON conversion utilities
//

std::unique_ptr<ContentId> ContentIdJsonToObj(const nlohmann::json& contentId, const ReportingHandler& handler);
std::unique_ptr<File> FileJsonToObj(const nlohmann::json& file, const ReportingHandler& handler);

//
// Comparison operators
//

/// @brief Compares two ContentId objects for equality. The values of members are strictly compared.
bool operator==(const ContentId& lhs, const ContentId& rhs);

/// @brief Compares two ContentId objects for inequality. The values of members are strictly compared.
bool operator!=(const ContentId& lhs, const ContentId& rhs);

/// @brief Compares two File objects for equality. The values of members are strictly compared.
bool operator==(const File& lhs, const File& rhs);

/// @brief Compares two File objects for inequality. The values of members are strictly compared.
bool operator!=(const File& lhs, const File& rhs);

/// @brief Compares two Content objects for equality. The values of members are strictly compared.
bool operator==(const Content& lhs, const Content& rhs);

/// @brief Compares two Content objects for inequality. The values of members are strictly compared.
bool operator!=(const Content& lhs, const Content& rhs);

/// @brief Compares two DeliveryOptimizationData objects for equality. The values of members are strictly compared.
bool operator==(const DeliveryOptimizationData& lhs, const DeliveryOptimizationData& rhs);

/// @brief Compares two DeliveryOptimizationData objects for inequality. The values of members are strictly compared.
bool operator!=(const DeliveryOptimizationData& lhs, const DeliveryOptimizationData& rhs);
} // namespace contentutil
} // namespace SFS::details
