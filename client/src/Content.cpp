// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Content.h"

#include "details/ErrorHandling.h"
#include "details/Util.h"

#include <algorithm>

using namespace SFS;
using namespace SFS::details::util;

Result ContentId::Make(std::string nameSpace,
                       std::string name,
                       std::string version,
                       std::unique_ptr<ContentId>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<ContentId> tmp(new ContentId());
    tmp->m_nameSpace = std::move(nameSpace);
    tmp->m_name = std::move(name);
    tmp->m_version = std::move(version);

    out = std::move(tmp);

    return Result::S_Ok;
}
SFS_CATCH_RETURN()

const std::string& ContentId::GetNameSpace() const noexcept
{
    return m_nameSpace;
}

const std::string& ContentId::GetName() const noexcept
{
    return m_name;
}

const std::string& ContentId::GetVersion() const noexcept
{
    return m_version;
}

bool ContentId::operator==(const ContentId& other) const noexcept
{
    return AreEqualI(m_nameSpace, other.m_nameSpace) && AreEqualI(m_name, other.m_name) &&
           AreEqualI(m_version, other.m_version);
}

bool ContentId::operator!=(const ContentId& other) const noexcept
{
    return !(*this == other);
}

Result File::Make(std::string fileId,
                  std::string url,
                  uint64_t sizeInBytes,
                  std::unordered_map<HashType, std::string> hashes,
                  std::unique_ptr<File>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<File> tmp(new File());
    tmp->m_fileId = std::move(fileId);
    tmp->m_url = std::move(url);
    tmp->m_sizeInBytes = sizeInBytes;
    tmp->m_hashes = std::move(hashes);

    out = std::move(tmp);

    return Result::S_Ok;
}
SFS_CATCH_RETURN()

Result File::Clone(std::unique_ptr<File>& out) noexcept
{
    return Make(m_fileId, m_url, m_sizeInBytes, m_hashes, out);
}

const std::string& File::GetFileId() const noexcept
{
    return m_fileId;
}

const std::string& File::GetUrl() const noexcept
{
    return m_url;
}

uint64_t File::GetSizeInBytes() const noexcept
{
    return m_sizeInBytes;
}

const std::unordered_map<HashType, std::string>& File::GetHashes() const noexcept
{
    return m_hashes;
}

bool File::operator==(const File& other) const noexcept
{
    return AreEqualI(m_fileId, other.m_fileId) && AreEqualI(m_url, other.m_url) &&
           m_sizeInBytes == other.m_sizeInBytes && m_hashes == other.m_hashes;
}

bool File::operator!=(const File& other) const noexcept
{
    return !(*this == other);
}

Result Content::Make(std::string contentNameSpace,
                     std::string contentName,
                     std::string contentVersion,
                     const std::vector<std::unique_ptr<File>>& files,
                     std::unique_ptr<Content>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<Content> tmp(new Content());
    RETURN_IF_FAILED(ContentId::Make(std::move(contentNameSpace),
                                     std::move(contentName),
                                     std::move(contentVersion),
                                     tmp->m_contentId));

    for (const auto& file : files)
    {
        std::unique_ptr<File> clone;
        RETURN_IF_FAILED(file->Clone(clone));
        tmp->m_files.push_back(std::move(clone));
    }

    out = std::move(tmp);

    return Result::S_Ok;
}
SFS_CATCH_RETURN()

Result Content::Make(std::string contentNameSpace,
                     std::string contentName,
                     std::string contentVersion,
                     std::vector<std::unique_ptr<File>>&& files,
                     std::unique_ptr<Content>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<Content> tmp(new Content());
    RETURN_IF_FAILED(ContentId::Make(std::move(contentNameSpace),
                                     std::move(contentName),
                                     std::move(contentVersion),
                                     tmp->m_contentId));
    tmp->m_files = std::move(files);

    out = std::move(tmp);

    return Result::S_Ok;
}
SFS_CATCH_RETURN()

const ContentId& Content::GetContentId() const noexcept
{
    return *m_contentId;
}

const std::vector<std::unique_ptr<File>>& Content::GetFiles() const noexcept
{
    return m_files;
}

bool Content::operator==(const Content& other) const noexcept
try
{
    return (m_contentId && other.m_contentId && *m_contentId == *other.m_contentId) &&
           (std::is_permutation(m_files.begin(),
                                m_files.end(),
                                other.m_files.begin(),
                                other.m_files.end(),
                                [](const std::unique_ptr<File>& lhs, const std::unique_ptr<File>& rhs) {
                                    return lhs && rhs && *lhs == *rhs;
                                }));
}
catch (...)
{
    return false;
}

bool Content::operator!=(const Content& other) const noexcept
{
    return !(*this == other);
}
