// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

namespace SFS::details
{
enum class ContentType
{
    Generic,
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
} // namespace SFS::details
