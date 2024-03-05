// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Content.h"
#include "SFSEntities.h"

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

std::unique_ptr<VersionEntity> ParseJsonToVersionEntity(const nlohmann::json& data, const ReportingHandler& handler);
std::unique_ptr<ContentId> GenericVersionEntityToContentId(VersionEntity&& entity, const ReportingHandler& handler);

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
} // namespace contentutil
} // namespace SFS::details
