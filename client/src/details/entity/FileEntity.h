// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ContentType.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace SFS::details
{
class ReportingHandler;

struct FileEntity
{
    virtual ~FileEntity()
    {
    }

    virtual ContentType GetContentType() const = 0;

    std::string fileId;
    std::string url;
    uint64_t sizeInBytes;
    std::unordered_map<std::string, std::string> hashes;

    static std::unique_ptr<FileEntity> FromJson(const nlohmann::json& file, const ReportingHandler& handler);
};

struct GenericFileEntity : public FileEntity
{
    ContentType GetContentType() const override;
};

using FileEntities = std::vector<std::unique_ptr<FileEntity>>;
} // namespace SFS::details
