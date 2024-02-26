// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "AppContent.h"

#include "details/ErrorHandling.h"

using namespace SFS;

AppFile::AppFile(std::string&& fileId,
                 std::string&& url,
                 uint64_t sizeInBytes,
                 std::unordered_map<HashType, std::string>&& hashes,
                 std::string&& fileMoniker)
    : File(std::move(fileId), std::move(url), sizeInBytes, std::move(hashes))
    , m_fileMoniker(std::move(fileMoniker))
{
}

Result AppFile::Make(std::string fileId,
                     std::string url,
                     uint64_t sizeInBytes,
                     std::unordered_map<HashType, std::string> hashes,
                     std::vector<Architecture> architectures,
                     std::vector<std::string> platformApplicabilityForPackage,
                     std::string fileMoniker,
                     std::unique_ptr<AppFile>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<ApplicabilityDetails> details;
    RETURN_IF_FAILED(
        ApplicabilityDetails::Make(std::move(architectures), std::move(platformApplicabilityForPackage), details));

    std::unique_ptr<AppFile> tmp(
        new AppFile(std::move(fileId), std::move(url), sizeInBytes, std::move(hashes), std::move(fileMoniker)));
    tmp->m_applicabilityDetails = std::move(details);

    out = std::move(tmp);

    return Result::Success;
}
SFS_CATCH_RETURN()

AppFile::AppFile(AppFile&& other) noexcept : File(std::move(other))
{
    m_applicabilityDetails = std::move(other.m_applicabilityDetails);
    m_fileMoniker = std::move(other.m_fileMoniker);
}

const ApplicabilityDetails& AppFile::GetApplicabilityDetails() const noexcept
{
    return *m_applicabilityDetails;
}

const std::string& AppFile::GetFileMoniker() const noexcept
{
    return m_fileMoniker;
}

Result AppContent::Make(std::unique_ptr<ContentId>&& contentId,
                        std::string updateId,
                        std::vector<Content>&& prerequisites,
                        std::vector<AppFile>&& files,
                        std::unique_ptr<AppContent>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<AppContent> tmp(new AppContent());
    tmp->m_contentId = std::move(contentId);
    tmp->m_updateId = std::move(updateId);
    tmp->m_prerequisites = std::move(prerequisites);
    tmp->m_files = std::move(files);

    out = std::move(tmp);

    return Result::Success;
}
SFS_CATCH_RETURN()

const ContentId& AppContent::GetContentId() const noexcept
{
    return *m_contentId;
}

const std::string& AppContent::GetUpdateId() const noexcept
{
    return m_updateId;
}

const std::vector<AppFile>& AppContent::GetFiles() const noexcept
{
    return m_files;
}

const std::vector<Content>& AppContent::GetPrerequisites() const noexcept
{
    return m_prerequisites;
}