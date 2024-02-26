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
class ContentId
{
  public:
    [[nodiscard]] static Result Make(std::string nameSpace,
                                     std::string name,
                                     std::string version,
                                     std::unique_ptr<ContentId>& out) noexcept;

    ContentId(ContentId&&) noexcept;

    ContentId(const ContentId&) = delete;
    ContentId& operator=(const ContentId&) = delete;

    /**
     * @return Content namespace
     */
    const std::string& GetNameSpace() const noexcept;

    /**
     * @return Content name
     */
    const std::string& GetName() const noexcept;

    /**
     * @return 4-part integer version. Each part can range from 0-65535
     */
    const std::string& GetVersion() const noexcept;

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

    File(File&&) noexcept;

    File(const File&) = delete;
    File& operator=(const File&) = delete;

    /**
     * @return Unique file identifier within a content version
     */
    const std::string& GetFileId() const noexcept;

    /**
     * @return Download URL
     */
    const std::string& GetUrl() const noexcept;

    /**
     * @return File size in number of bytes
     */
    uint64_t GetSizeInBytes() const noexcept;

    /**
     * @return Dictionary of algorithm type to base64 encoded file hash string
     */
    const std::unordered_map<HashType, std::string>& GetHashes() const noexcept;

  protected:
    File(std::string&& fileId,
         std::string&& url,
         uint64_t sizeInBytes,
         std::unordered_map<HashType, std::string>&& hashes);

    [[nodiscard]] Result Clone(std::unique_ptr<File>& out) const noexcept;

    friend class Content;

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
                                     const std::vector<File>& files,
                                     std::unique_ptr<Content>& out) noexcept;

    /**
     * @brief This Make() method should be used when the caller wants the @param files to be moved
     */
    [[nodiscard]] static Result Make(std::string contentNameSpace,
                                     std::string contentName,
                                     std::string contentVersion,
                                     std::vector<File>&& files,
                                     std::unique_ptr<Content>& out) noexcept;

    /**
     * @brief This Make() method should be used when the caller wants the @param contentId and @param files to be moved
     */
    [[nodiscard]] static Result Make(std::unique_ptr<ContentId>&& contentId,
                                     std::vector<File>&& files,
                                     std::unique_ptr<Content>& out) noexcept;

    Content(const Content&) = delete;
    Content& operator=(const Content&) = delete;

    /**
     * @return Unique content identifier
     */
    const ContentId& GetContentId() const noexcept;

    const std::vector<File>& GetFiles() const noexcept;

  private:
    Content() = default;

    std::unique_ptr<ContentId> m_contentId;
    std::vector<File> m_files;
};
} // namespace SFS
