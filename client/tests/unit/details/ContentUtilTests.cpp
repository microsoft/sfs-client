// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../util/SFSExceptionMatcher.h"
#include "../../util/TestHelper.h"
#include "ContentUtil.h"
#include "ReportingHandler.h"
#include "entity/FileEntity.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#define TEST(...) TEST_CASE("[FileEntityTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;
using namespace SFS::details::contentutil;
using namespace SFS::test;
using json = nlohmann::json;

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
            REQUIRE_NOTHROW(files = GenericFileEntitiesToFileVector(std::move(entities), handler));
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
            REQUIRE_NOTHROW(files = GenericFileEntitiesToFileVector(std::move(entities), handler));
            REQUIRE(files.size() == 2);
            CheckFile(files[0]);
            CheckFile(files[1]);
        }
    }
}
