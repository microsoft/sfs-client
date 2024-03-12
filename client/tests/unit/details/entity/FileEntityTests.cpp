// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../../util/SFSExceptionMatcher.h"
#include "../../../util/TestHelper.h"
#include "ReportingHandler.h"
#include "entity/FileEntity.h"

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
}
