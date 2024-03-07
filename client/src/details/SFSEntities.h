// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

namespace SFS::details
{
enum class ContentType
{
    Generic,
    App,
};

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
};

struct GenericVersionEntity : public VersionEntity
{
    ContentType GetContentType() const override
    {
        return ContentType::Generic;
    }
};

struct AppVersionEntity : public GenericVersionEntity
{
    ContentType GetContentType() const override
    {
        return ContentType::App;
    }

    std::string updateId;
    std::vector<GenericVersionEntity> prerequisites;
};

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
};

struct GenericFileEntity : public FileEntity
{
    ContentType GetContentType() const override
    {
        return ContentType::Generic;
    }
};
} // namespace SFS::details
