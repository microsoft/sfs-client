// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace SFS
{
using SearchAttributes = std::unordered_map<std::string, std::string>;

class ContentId
{
  public:
    [[nodiscard]] static Result Make(std::string nameSpace,
                                     std::string name,
                                     std::string version,
                                     std::unique_ptr<ContentId>& out) noexcept;

    ContentId(const ContentId&) = delete;
    ContentId& operator=(const ContentId&) = delete;

    const std::string& GetNameSpace() const noexcept;
    const std::string& GetName() const noexcept;
    const std::string& GetVersion() const noexcept;

    bool operator==(const ContentId& other) const noexcept;
    bool operator!=(const ContentId& other) const noexcept;

  private:
    ContentId() = default;

    std::string m_nameSpace;
    std::string m_name;
    std::string m_version;
};

enum class HashType
{
    Sha1,
    Sha256
};

class File
{
  public:
    [[nodiscard]] static Result Make(std::string fileId,
                                     std::string url,
                                     uint64_t sizeInBytes,
                                     std::unordered_map<HashType, std::string> hashes,
                                     std::unique_ptr<File>& out) noexcept;

    [[nodiscard]] Result Clone(std::unique_ptr<File>& out) noexcept;

    File(const File&) = delete;
    File& operator=(const File&) = delete;

    const std::string& GetFileId() const noexcept;
    const std::string& GetUrl() const noexcept;
    uint64_t GetSizeInBytes() const noexcept;
    const std::unordered_map<HashType, std::string>& GetHashes() const noexcept;

    bool operator==(const File& other) const noexcept;
    bool operator!=(const File& other) const noexcept;

  private:
    File() = default;

    std::string m_fileId;
    std::string m_url;
    uint64_t m_sizeInBytes;
    std::unordered_map<HashType, std::string> m_hashes;
};

class Content
{
  public:
    /**
     * @brief This Make() method should be used when the caller wants the @param files to be cloned
     */
    [[nodiscard]] static Result Make(std::string contentNameSpace,
                                     std::string contentName,
                                     std::string contentVersion,
                                     const std::vector<std::unique_ptr<File>>& files,
                                     std::unique_ptr<Content>& out) noexcept;

    /**
     * @brief This Make() method should be used when the caller wants the @param files to be moved
     */
    [[nodiscard]] static Result Make(std::string contentNameSpace,
                                     std::string contentName,
                                     std::string contentVersion,
                                     std::vector<std::unique_ptr<File>>&& files,
                                     std::unique_ptr<Content>& out) noexcept;

    Content(const Content&) = delete;
    Content& operator=(const Content&) = delete;

    const ContentId& GetContentId() const noexcept;

    const std::vector<std::unique_ptr<File>>& GetFiles() const noexcept;

    bool operator==(const Content& other) const noexcept;
    bool operator!=(const Content& other) const noexcept;

  private:
    Content() = default;

    std::unique_ptr<ContentId> m_contentId;
    std::vector<std::unique_ptr<File>> m_files;
};
} // namespace SFS
