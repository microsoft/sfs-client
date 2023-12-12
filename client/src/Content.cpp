// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Content.h"

#include "details/ErrorHandling.h"

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
                     std::string correlationVector,
                     std::vector<std::unique_ptr<File>>& files,
                     std::unique_ptr<Content>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<Content> tmp(new Content());
    RETURN_IF_FAILED(ContentId::Make(std::move(contentNameSpace),
                                     std::move(contentName),
                                     std::move(contentVersion),
                                     tmp->m_contentId));
    tmp->m_correlationVector = std::move(correlationVector);
    tmp->m_files = std::move(files);

    out = std::move(tmp);

    return Result::S_Ok;
}
SFS_CATCH_RETURN()

const ContentId& Content::GetContentId() const noexcept
{
    return *m_contentId;
}

const std::string& Content::GetCorrelationVector() const noexcept
{
    return m_correlationVector;
}

const std::vector<std::unique_ptr<File>>& Content::GetFiles() const noexcept
{
    return m_files;
}
