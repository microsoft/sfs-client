// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ApplicabilityDetails.h"
#include "Content.h"

#include <memory>
#include <string>
#include <vector>

namespace SFS
{
class AppFile : private File
{
  public:
    [[nodiscard]] static Result Make(std::string fileId,
                                     std::string url,
                                     uint64_t sizeInBytes,
                                     std::unordered_map<HashType, std::string> hashes,
                                     std::vector<Architecture> architectures,
                                     std::vector<std::string> platformApplicabilityForPackage,
                                     std::string fileMoniker,
                                     std::unique_ptr<AppFile>& out) noexcept;

    AppFile(AppFile&&) noexcept;

    AppFile(const AppFile&) = delete;
    AppFile& operator=(const AppFile&) = delete;

    /// Getter methods from File class
    using File::GetFileId;
    using File::GetHashes;
    using File::GetSizeInBytes;
    using File::GetUrl;

    /**
     * @return Set of details related to applicability of the file
     */
    const ApplicabilityDetails& GetApplicabilityDetails() const noexcept;

    /**
     * @return Package Moniker of the file
     */
    const std::string& GetFileMoniker() const noexcept;

  private:
    AppFile(std::string&& fileId,
            std::string&& url,
            uint64_t sizeInBytes,
            std::unordered_map<HashType, std::string>&& hashes,
            std::string&& fileMoniker);

    std::unique_ptr<ApplicabilityDetails> m_applicabilityDetails;
    std::string m_fileMoniker;
};

class AppContent
{
  public:
    [[nodiscard]] static Result Make(std::unique_ptr<ContentId>&& contentId,
                                     std::string updateId,
                                     std::vector<Content>&& prerequisites,
                                     std::vector<AppFile>&& files,
                                     std::unique_ptr<AppContent>& out) noexcept;

    AppContent(AppContent&&) noexcept;

    AppContent(const AppContent&) = delete;
    AppContent& operator=(const AppContent&) = delete;

    /**
     * @return Unique content identifier
     */
    const ContentId& GetContentId() const noexcept;

    /**
     * @return Unique Update Id
     */
    const std::string& GetUpdateId() const noexcept;

    /**
     * @return Files belonging to this App
     */
    const std::vector<AppFile>& GetFiles() const noexcept;

    /**
     * @return List of Prerequisite content needed for this App. Prerequisites don't have further dependencies.
     */
    const std::vector<Content>& GetPrerequisites() const noexcept;

  private:
    AppContent() = default;

    std::unique_ptr<ContentId> m_contentId;
    std::vector<AppFile> m_files;

    std::string m_updateId;
    std::vector<Content> m_prerequisites;
};
} // namespace SFS
