// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ContentUtil.h"

#include "ErrorHandling.h"
#include "ReportingHandler.h"
#include "Util.h"

#include <nlohmann/json.hpp>

using namespace SFS;
using namespace SFS::details;
using namespace SFS::details::util;
using json = nlohmann::json;

namespace
{
HashType HashTypeFromString(const std::string& hashType, const ReportingHandler& handler)
{
    if (AreEqualI(hashType, "Sha1"))
    {
        return HashType::Sha1;
    }
    else if (AreEqualI(hashType, "Sha256"))
    {
        return HashType::Sha256;
    }
    else
    {
        THROW_LOG(Result(Result::Unexpected, "Unknown hash type: " + hashType), handler);
        return HashType::Sha1; // Unreachable code, but the compiler doesn't know that.
    }
}

template <typename Entity>
void ValidateContentType(const Entity& entity, ContentType expectedType, const ReportingHandler& handler)
{
    THROW_CODE_IF_LOG(Result::ServiceUnexpectedContentType,
                      entity.GetContentType() != expectedType,
                      handler,
                      "Unexpected content type returned by the service");
}

std::unique_ptr<File> GenericFileEntityToFile(FileEntity&& entity, const ReportingHandler& handler)
{
    ValidateContentType(entity, ContentType::Generic, handler);

    std::unordered_map<HashType, std::string> hashes;
    for (auto& [hashType, hashValue] : entity.hashes)
    {
        hashes[HashTypeFromString(hashType, handler)] = std::move(hashValue);
    }

    std::unique_ptr<File> tmp;
    THROW_IF_FAILED_LOG(
        File::Make(std::move(entity.fileId), std::move(entity.url), entity.sizeInBytes, std::move(hashes), tmp),
        handler);
    return tmp;
}
} // namespace

std::unique_ptr<ContentId> contentutil::GenericVersionEntityToContentId(VersionEntity&& entity,
                                                                        const ReportingHandler& handler)
{
    ValidateContentType(entity, ContentType::Generic, handler);

    std::unique_ptr<ContentId> tmp;
    THROW_IF_FAILED_LOG(ContentId::Make(std::move(entity.contentId.nameSpace),
                                        std::move(entity.contentId.name),
                                        std::move(entity.contentId.version),
                                        tmp),
                        handler);
    return tmp;
}

std::vector<File> contentutil::GenericFileEntitiesToFileVector(FileEntities&& entities, const ReportingHandler& handler)
{
    std::vector<File> tmp;
    for (auto& entity : entities)
    {
        tmp.push_back(std::move(*GenericFileEntityToFile(std::move(*entity), handler)));
    }

    return tmp;
}

FileEntities contentutil::DownloadInfoResponseToFileEntities(const json& data, const ReportingHandler& handler)
{
    // Expected format is an array of FileEntity
    THROW_CODE_IF_NOT_LOG(ServiceInvalidResponse, data.is_array(), handler, "Response is not a JSON array");

    FileEntities tmp;
    for (const auto& fileData : data)
    {
        THROW_CODE_IF_NOT_LOG(ServiceInvalidResponse,
                              fileData.is_object(),
                              handler,
                              "Array element is not a JSON object");
        tmp.push_back(std::move(FileEntity::FromJson(fileData, handler)));
    }

    return tmp;
}

bool contentutil::operator==(const ContentId& lhs, const ContentId& rhs)
{
    // String characters can be UTF-8 encoded, so we need to compare them in a case-sensitive manner.
    return lhs.GetNameSpace() == rhs.GetNameSpace() && lhs.GetName() == rhs.GetName() &&
           lhs.GetVersion() == rhs.GetVersion();
}

bool contentutil::operator!=(const ContentId& lhs, const ContentId& rhs)
{
    return !(lhs == rhs);
}

bool contentutil::operator==(const File& lhs, const File& rhs)
{
    // String characters can be UTF-8 encoded, so we need to compare them in a case-sensitive manner.
    return lhs.GetFileId() == rhs.GetFileId() && lhs.GetUrl() == rhs.GetUrl() &&
           lhs.GetSizeInBytes() == rhs.GetSizeInBytes() && lhs.GetHashes() == rhs.GetHashes();
}

bool contentutil::operator!=(const File& lhs, const File& rhs)
{
    return !(lhs == rhs);
}

bool contentutil::operator==(const ApplicabilityDetails& lhs, const ApplicabilityDetails& rhs)
{
    // String characters can be UTF-8 encoded, so we need to compare them in a case-sensitive manner.
    return lhs.GetArchitectures() == rhs.GetArchitectures() &&
           lhs.GetPlatformApplicabilityForPackage() == rhs.GetPlatformApplicabilityForPackage();
}

bool contentutil::operator!=(const ApplicabilityDetails& lhs, const ApplicabilityDetails& rhs)
{
    return !(lhs == rhs);
}

bool contentutil::operator==(const AppFile& lhs, const AppFile& rhs)
{
    // String characters can be UTF-8 encoded, so we need to compare them in a case-sensitive manner.
    return lhs.GetFileId() == rhs.GetFileId() && lhs.GetUrl() == rhs.GetUrl() &&
           lhs.GetSizeInBytes() == rhs.GetSizeInBytes() && lhs.GetHashes() == rhs.GetHashes() &&
           lhs.GetApplicabilityDetails() == rhs.GetApplicabilityDetails() &&
           lhs.GetFileMoniker() == rhs.GetFileMoniker();
}

bool contentutil::operator!=(const AppFile& lhs, const AppFile& rhs)
{
    return !(lhs == rhs);
}

bool contentutil::operator==(const Content& lhs, const Content& rhs)
{
    return lhs.GetContentId() == rhs.GetContentId() &&
           (std::is_permutation(lhs.GetFiles().begin(),
                                lhs.GetFiles().end(),
                                rhs.GetFiles().begin(),
                                rhs.GetFiles().end(),
                                [](const File& flhs, const File& frhs) { return flhs == frhs; }));
}

bool contentutil::operator!=(const Content& lhs, const Content& rhs)
{
    return !(lhs == rhs);
}

bool contentutil::operator==(const AppPrerequisiteContent& lhs, const AppPrerequisiteContent& rhs)
{
    auto areFilesEqual = [&lhs, &rhs]() {
        return std::is_permutation(lhs.GetFiles().begin(),
                                   lhs.GetFiles().end(),
                                   rhs.GetFiles().begin(),
                                   rhs.GetFiles().end(),
                                   [](const AppFile& flhs, const AppFile& frhs) { return flhs == frhs; });
    };
    return lhs.GetContentId() == rhs.GetContentId() && areFilesEqual();
}

bool contentutil::operator!=(const AppPrerequisiteContent& lhs, const AppPrerequisiteContent& rhs)
{
    return !(lhs == rhs);
}

bool contentutil::operator==(const AppContent& lhs, const AppContent& rhs)
{
    auto arePrerequisitesEqual = [&lhs, &rhs]() {
        return std::equal(
            lhs.GetPrerequisites().begin(),
            lhs.GetPrerequisites().end(),
            rhs.GetPrerequisites().begin(),
            rhs.GetPrerequisites().end(),
            [](const AppPrerequisiteContent& clhs, const AppPrerequisiteContent& crhs) { return clhs == crhs; });
    };
    auto areFilesEqual = [&lhs, &rhs]() {
        return std::is_permutation(lhs.GetFiles().begin(),
                                   lhs.GetFiles().end(),
                                   rhs.GetFiles().begin(),
                                   rhs.GetFiles().end(),
                                   [](const AppFile& flhs, const AppFile& frhs) { return flhs == frhs; });
    };
    return lhs.GetContentId() == rhs.GetContentId() && lhs.GetUpdateId() == rhs.GetUpdateId() &&
           arePrerequisitesEqual() && areFilesEqual();
}

bool contentutil::operator!=(const AppContent& lhs, const AppContent& rhs)
{
    return !(lhs == rhs);
}
