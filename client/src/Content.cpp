// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Content.h"

#include "details/ErrorHandling.h"

#include <algorithm>

using namespace SFS;

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

    return Result::Success;
}
SFS_CATCH_RETURN()

ContentId::ContentId(ContentId&& other) noexcept
{
    m_nameSpace = std::move(other.m_nameSpace);
    m_name = std::move(other.m_name);
    m_version = std::move(other.m_version);
}

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

File::File(std::string&& fileId,
           std::string&& url,
           uint64_t sizeInBytes,
           std::unordered_map<HashType, std::string>&& hashes)
    : m_fileId(std::move(fileId))
    , m_url(std::move(url))
    , m_sizeInBytes(sizeInBytes)
    , m_hashes(std::move(hashes))
{
}

Result File::Make(std::string fileId,
                  std::string url,
                  uint64_t sizeInBytes,
                  std::unordered_map<HashType, std::string> hashes,
                  std::unique_ptr<File>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<File> tmp(new File(std::move(fileId), std::move(url), sizeInBytes, std::move(hashes)));
    out = std::move(tmp);

    return Result::Success;
}
SFS_CATCH_RETURN()

Result File::Clone(std::unique_ptr<File>& out) const noexcept
{
    return Make(m_fileId, m_url, m_sizeInBytes, m_hashes, out);
}

File::File(File&& other) noexcept
{
    m_fileId = std::move(other.m_fileId);
    m_url = std::move(other.m_url);
    m_sizeInBytes = other.m_sizeInBytes;
    m_hashes = std::move(other.m_hashes);
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

Result Content::Make(std::string contentNameSpace,
                     std::string contentName,
                     std::string contentVersion,
                     const std::vector<File>& files,
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
        RETURN_IF_FAILED(file.Clone(clone));
        tmp->m_files.push_back(std::move(*clone));
    }

    out = std::move(tmp);

    return Result::Success;
}
SFS_CATCH_RETURN()

Result Content::Make(std::string contentNameSpace,
                     std::string contentName,
                     std::string contentVersion,
                     std::vector<File>&& files,
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

    return Result::Success;
}
SFS_CATCH_RETURN()

Result Content::Make(std::unique_ptr<ContentId>&& contentId,
                     std::vector<File>&& files,
                     std::unique_ptr<Content>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<Content> tmp(new Content());
    tmp->m_contentId = std::move(contentId);
    tmp->m_files = std::move(files);

    out = std::move(tmp);

    return Result::Success;
}
SFS_CATCH_RETURN()

const ContentId& Content::GetContentId() const noexcept
{
    return *m_contentId;
}

const std::vector<File>& Content::GetFiles() const noexcept
{
    return m_files;
}
