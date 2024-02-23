// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "File.h"
#include "Result.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace SFS
{
using SearchAttributes = std::unordered_map<std::string, std::string>;
using ProductRequest = std::pair<std::string, SearchAttributes>;

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
