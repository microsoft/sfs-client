// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ContentType.h"

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace SFS::details
{
class ReportingHandler;

struct ContentIdEntity
{
    std::string nameSpace;
    std::string name;
    std::string version;
};

struct VersionEntity
{
    virtual ~VersionEntity()
    {
    }

    virtual ContentType GetContentType() const = 0;

    ContentIdEntity contentId;

    static std::unique_ptr<VersionEntity> FromJson(const nlohmann::json& data, const ReportingHandler& handler);
};

struct GenericVersionEntity : public VersionEntity
{
    ContentType GetContentType() const override;
};

struct AppVersionEntity : public VersionEntity
{
    ContentType GetContentType() const override;

    std::string updateId;
    std::vector<GenericVersionEntity> prerequisites;
};

using VersionEntities = std::vector<std::unique_ptr<VersionEntity>>;
} // namespace SFS::details
