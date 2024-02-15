// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Content.h"

#include <nlohmann/json_fwd.hpp>

#include <memory>

namespace SFS::details
{
class ReportingHandler;

namespace contentutil
{
std::unique_ptr<ContentId> ContentIdJsonToObj(const nlohmann::json& contentId, const ReportingHandler& handler);
std::unique_ptr<File> FileJsonToObj(const nlohmann::json& file, const ReportingHandler& handler);
} // namespace contentutil

} // namespace SFS::details
