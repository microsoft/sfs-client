// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../../util/SFSExceptionMatcher.h"
#include "../../../util/TestHelper.h"
#include "ReportingHandler.h"
#include "entity/FileEntity.h"
#include "sfsclient/File.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#define TEST(...) TEST_CASE("[FileEntityTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;
using namespace SFS::test;
using json = nlohmann::json;

TEST("Testing FileEntity::FromJson()")
{
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);
    const std::string fileId = "fileId";
    const std::string url = "url";
    const uint64_t size = 123;
    const std::string sha1 = "sha1";
    const std::string sha256 = "sha256";

    SECTION("Generic File Entity")
    {
        std::unique_ptr<FileEntity> entity;
        SECTION("Correct")
        {
            const json fileEntity = {{"FileId", fileId},
                                     {"Url", url},
                                     {"SizeInBytes", size},
                                     {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};

            REQUIRE_NOTHROW(entity = FileEntity::FromJson(fileEntity, handler));
            REQUIRE(entity != nullptr);
            REQUIRE(entity->GetContentType() == ContentType::Generic);
            REQUIRE(entity->fileId == fileId);
            REQUIRE(entity->url == url);
            REQUIRE(entity->sizeInBytes == size);
            REQUIRE(entity->hashes.size() == 2);
            REQUIRE(entity->hashes.at("Sha1") == sha1);
            REQUIRE(entity->hashes.at("Sha256") == sha256);
        }

        SECTION("Missing fields")
        {
            SECTION("Missing FileId")
            {
                const json fileEntity = {{"Url", url},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing File.FileId in response");
            }

            SECTION("Missing Url")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing File.Url in response");
            }

            SECTION("Missing SizeInBytes")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing File.SizeInBytes in response");
            }

            SECTION("Missing Hashes")
            {
                const json fileEntity = {{"FileId", fileId}, {"Url", url}, {"SizeInBytes", size}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing File.Hashes in response");
            }
        }

        SECTION("Wrong types")
        {
            SECTION("FileId")
            {
                const json fileEntity = {{"FileId", 1},
                                         {"Url", url},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.FileId is not a string");
            }

            SECTION("Url")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", 1},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.Url is not a string");
            }

            SECTION("SizeInBytes")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"SizeInBytes", "size"},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.SizeInBytes is not an unsigned number");
            }

            SECTION("Hashes not an object")
            {
                const json fileEntity = {{"FileId", fileId}, {"Url", url}, {"SizeInBytes", size}, {"Hashes", 1}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.Hashes is not an object");
            }

            SECTION("Hashes.Sha1")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", 1}}}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.Hashes object value is not a string");
            }
        }
    }

    SECTION("App File Entity")
    {
        const std::string fileMoniker = "fileMoniker";
        std::unique_ptr<FileEntity> entity;
        const std::string arch = "amd64";
        const std::string applicability = "app";
        const json applicabilityDetails = {{"Architectures", json::array({arch})},
                                           {"PlatformApplicabilityForPackage", json::array({applicability})}};

        SECTION("Correct")
        {
            const json fileEntity = {{"FileId", fileId},
                                     {"Url", url},
                                     {"SizeInBytes", size},
                                     {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                     {"FileMoniker", fileMoniker},
                                     {"ApplicabilityDetails", applicabilityDetails}};

            REQUIRE_NOTHROW(entity = FileEntity::FromJson(fileEntity, handler));
            REQUIRE(entity != nullptr);
            REQUIRE(entity->GetContentType() == ContentType::App);
            REQUIRE(entity->fileId == fileId);
            REQUIRE(entity->url == url);
            REQUIRE(entity->sizeInBytes == size);
            REQUIRE(entity->hashes.size() == 2);
            REQUIRE(entity->hashes.at("Sha1") == sha1);
            REQUIRE(entity->hashes.at("Sha256") == sha256);

            AppFileEntity* appEntity = dynamic_cast<AppFileEntity*>(entity.get());
            REQUIRE(appEntity->fileMoniker == fileMoniker);
            REQUIRE(appEntity->applicabilityDetails.architectures.size() == 1);
            REQUIRE(appEntity->applicabilityDetails.architectures[0] == arch);
            REQUIRE(appEntity->applicabilityDetails.platformApplicabilityForPackage.size() == 1);
            REQUIRE(appEntity->applicabilityDetails.platformApplicabilityForPackage[0] == applicability);
        }

        SECTION("Missing fields")
        {
            SECTION("Missing ApplicabilityDetails")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                         {"FileMoniker", fileMoniker}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing File.ApplicabilityDetails in response");
            }

            SECTION("Missing ApplicabilityDetails.Architectures")
            {
                const json wrongApplicabilityDetails = {
                    {"PlatformApplicabilityForPackage", json::array({applicability})}};
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                         {"FileMoniker", fileMoniker},
                                         {"ApplicabilityDetails", wrongApplicabilityDetails}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing File.ApplicabilityDetails.Architectures in response");
            }

            SECTION("Missing ApplicabilityDetails.PlatformApplicabilityForPackage")
            {
                const json wrongApplicabilityDetails = {{"Architectures", json::array({arch})}};
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                         {"FileMoniker", fileMoniker},
                                         {"ApplicabilityDetails", wrongApplicabilityDetails}};
                REQUIRE_THROWS_CODE_MSG(
                    FileEntity::FromJson(fileEntity, handler),
                    ServiceInvalidResponse,
                    "Missing File.ApplicabilityDetails.PlatformApplicabilityForPackage in response");
            }
        }

        SECTION("Wrong types")
        {
            SECTION("FileMoniker")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                         {"FileMoniker", 1},
                                         {"ApplicabilityDetails", applicabilityDetails}};
                REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.FileMoniker is not a string");
            }

            SECTION("ApplicabilityDetails")
            {
                SECTION("Not an object")
                {
                    const json fileEntity = {{"FileId", fileId},
                                             {"Url", url},
                                             {"SizeInBytes", size},
                                             {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                             {"FileMoniker", fileMoniker},
                                             {"ApplicabilityDetails", fileId}};
                    REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                            ServiceInvalidResponse,
                                            "File.ApplicabilityDetails is not an object");
                }

                SECTION("Architectures is not an array")
                {
                    const json wrongApplicabilityDetails = {
                        {"Architectures", fileId},
                        {"PlatformApplicabilityForPackage", json::array({applicability})}};
                    const json fileEntity = {{"FileId", fileId},
                                             {"Url", url},
                                             {"SizeInBytes", size},
                                             {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                             {"FileMoniker", fileMoniker},
                                             {"ApplicabilityDetails", wrongApplicabilityDetails}};
                    REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                            ServiceInvalidResponse,
                                            "File.ApplicabilityDetails.Architectures is not an array");
                }

                SECTION("Architectures array value is not a string")
                {
                    const json wrongApplicabilityDetails = {
                        {"Architectures", json::array({1})},
                        {"PlatformApplicabilityForPackage", json::array({applicability})}};
                    const json fileEntity = {{"FileId", fileId},
                                             {"Url", url},
                                             {"SizeInBytes", size},
                                             {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                             {"FileMoniker", fileMoniker},
                                             {"ApplicabilityDetails", wrongApplicabilityDetails}};
                    REQUIRE_THROWS_CODE_MSG(FileEntity::FromJson(fileEntity, handler),
                                            ServiceInvalidResponse,
                                            "File.ApplicabilityDetails.Architectures array value is not a string");
                }

                SECTION("PlatformApplicabilityForPackage is not an array")
                {
                    const json wrongApplicabilityDetails = {{"Architectures", json::array({arch})},
                                                            {"PlatformApplicabilityForPackage", fileId}};
                    const json fileEntity = {{"FileId", fileId},
                                             {"Url", url},
                                             {"SizeInBytes", size},
                                             {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                             {"FileMoniker", fileMoniker},
                                             {"ApplicabilityDetails", wrongApplicabilityDetails}};
                    REQUIRE_THROWS_CODE_MSG(
                        FileEntity::FromJson(fileEntity, handler),
                        ServiceInvalidResponse,
                        "File.ApplicabilityDetails.PlatformApplicabilityForPackage is not an array");
                }

                SECTION("PlatformApplicabilityForPackage array value is not a string")
                {
                    const json wrongApplicabilityDetails = {{"Architectures", json::array({arch})},
                                                            {"PlatformApplicabilityForPackage", json::array({1})}};
                    const json fileEntity = {{"FileId", fileId},
                                             {"Url", url},
                                             {"SizeInBytes", size},
                                             {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}},
                                             {"FileMoniker", fileMoniker},
                                             {"ApplicabilityDetails", wrongApplicabilityDetails}};
                    REQUIRE_THROWS_CODE_MSG(
                        FileEntity::FromJson(fileEntity, handler),
                        ServiceInvalidResponse,
                        "File.ApplicabilityDetails.PlatformApplicabilityForPackage array value is not a string");
                }
            }
        }
    }
}

TEST("Testing GenericFileEntitiesToFileVector()")
{
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);

    const std::string fileId = "fileId";
    const std::string url = "url";
    const uint64_t size = 123;
    const std::string sha1 = "sha1";
    const std::string sha256 = "sha256";

    SECTION("Generic File Entity")
    {
        std::unique_ptr<FileEntity> entity = std::make_unique<GenericFileEntity>();
        entity->fileId = fileId;
        entity->url = url;
        entity->sizeInBytes = size;
        entity->hashes = {{"Sha1", sha1}, {"Sha256", sha256}};

        std::vector<std::unique_ptr<FileEntity>> entities;
        entities.push_back(std::move(entity));

        auto CheckFile = [&](const File& file) {
            REQUIRE(file.GetFileId() == fileId);
            REQUIRE(file.GetUrl() == url);
            REQUIRE(file.GetSizeInBytes() == size);
            REQUIRE(file.GetHashes().size() == 2);
            REQUIRE(file.GetHashes().at(HashType::Sha1) == sha1);
            REQUIRE(file.GetHashes().at(HashType::Sha256) == sha256);
        };

        std::vector<File> files;
        SECTION("1 file")
        {
            REQUIRE_NOTHROW(files = GenericFileEntity::FileEntitiesToFileVector(std::move(entities), handler));
            REQUIRE(files.size() == 1);
            CheckFile(files[0]);
        }

        SECTION("2 files")
        {
            std::unique_ptr<FileEntity> entity2 = std::make_unique<GenericFileEntity>();
            entity2->fileId = fileId;
            entity2->url = url;
            entity2->sizeInBytes = size;
            entity2->hashes = {{"Sha1", sha1}, {"Sha256", sha256}};

            entities.push_back(std::move(entity2));
            REQUIRE_NOTHROW(files = GenericFileEntity::FileEntitiesToFileVector(std::move(entities), handler));
            REQUIRE(files.size() == 2);
            CheckFile(files[0]);
            CheckFile(files[1]);
        }
    }
}
