// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../util/SFSExceptionMatcher.h"
#include "../../util/TestHelper.h"
#include "ContentUtil.h"
#include "ReportingHandler.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#define TEST(...) TEST_CASE("[ContentUtilTests] " __VA_ARGS__)

using namespace SFS::details;
using namespace SFS::details::contentutil;
using namespace SFS::test;
using json = nlohmann::json;

TEST("Testing ParseJsonToVersionEntity()")
{
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);
    const std::string ns = "namespace";
    const std::string name = "name";
    const std::string version = "version";

    SECTION("Generic Version Entity")
    {
        std::unique_ptr<VersionEntity> entity;
        SECTION("Correct")
        {
            const json versionEntity = {{"ContentId", {{"Namespace", ns}, {"Name", name}, {"Version", version}}}};

            REQUIRE_NOTHROW(entity = ParseJsonToVersionEntity(versionEntity, handler));
            REQUIRE(entity != nullptr);
            REQUIRE(entity->GetContentType() == ContentType::Generic);
            REQUIRE(entity->contentId.nameSpace == ns);
            REQUIRE(entity->contentId.name == name);
            REQUIRE(entity->contentId.version == version);
        }

        SECTION("Missing fields")
        {
            SECTION("Missing ContentId")
            {
                const json versionEntity = {{"Namespace", ns}, {"Name", name}, {"Version", version}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing ContentId in response");
            }

            SECTION("Missing ContentId.Namespace")
            {
                const json versionEntity = {{"ContentId", {{"Name", name}, {"Version", version}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing ContentId.Namespace in response");
            }

            SECTION("Missing ContentId.Name")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", ns}, {"Version", version}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing ContentId.Name in response");
            }

            SECTION("Missing ContentId.Version")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", ns}, {"Name", name}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing ContentId.Version in response");
            }
        }

        SECTION("Wrong types")
        {
            SECTION("ContentId not an object")
            {
                const json versionEntity = json::array({{"Namespace", 1}, {"Name", name}, {"Version", version}});
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Response is not a JSON object");
            }

            SECTION("ContentId.Namespace")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", 1}, {"Name", name}, {"Version", version}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "ContentId.Namespace is not a string");
            }

            SECTION("ContentId.Name")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", ns}, {"Name", 1}, {"Version", version}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "ContentId.Name is not a string");
            }

            SECTION("ContentId.Version")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", ns}, {"Name", name}, {"Version", 1}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "ContentId.Version is not a string");
            }
        }
    }

    SECTION("App Version Entity")
    {
        const std::string updateId = "updateId";
        std::unique_ptr<VersionEntity> entity;
        const json contentId = {{"Namespace", ns}, {"Name", name}, {"Version", version}};

        SECTION("Correct")
        {
            const json versionEntity = {{"ContentId", contentId},
                                        {"UpdateId", updateId},
                                        {"Prerequisites", json::array({contentId})}};

            REQUIRE_NOTHROW(entity = ParseJsonToVersionEntity(versionEntity, handler));
            REQUIRE(entity != nullptr);
            REQUIRE(entity->GetContentType() == ContentType::App);
            REQUIRE(entity->contentId.nameSpace == ns);
            REQUIRE(entity->contentId.name == name);
            REQUIRE(entity->contentId.version == version);

            AppVersionEntity* appEntity = dynamic_cast<AppVersionEntity*>(entity.get());
            REQUIRE(appEntity->updateId == updateId);
            REQUIRE(appEntity->prerequisites.size() == 1);
            REQUIRE(appEntity->prerequisites[0].contentId.nameSpace == ns);
            REQUIRE(appEntity->prerequisites[0].contentId.name == name);
            REQUIRE(appEntity->prerequisites[0].contentId.version == version);
        }

        SECTION("Missing fields")
        {
            SECTION("Missing Prerequisites")
            {
                const json versionEntity = {{"ContentId", contentId}, {"UpdateId", updateId}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing Prerequisites in response");
            }

            SECTION("Missing Prerequisite.Namespace")
            {
                const json wrongPrerequisite = {{"Name", name}, {"Version", version}};
                const json versionEntity = {{"ContentId", contentId},
                                            {"UpdateId", updateId},
                                            {"Prerequisites", json::array({wrongPrerequisite})}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing Prerequisite.Namespace in response");
            }

            SECTION("Missing Prerequisite.Name")
            {
                const json wrongPrerequisite = {{"Namespace", ns}, {"Version", version}};
                const json versionEntity = {{"ContentId", contentId},
                                            {"UpdateId", updateId},
                                            {"Prerequisites", json::array({wrongPrerequisite})}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing Prerequisite.Name in response");
            }

            SECTION("Missing Prerequisite.Version")
            {
                const json wrongPrerequisite = {{"Namespace", ns}, {"Name", name}};
                const json versionEntity = {{"ContentId", contentId},
                                            {"UpdateId", updateId},
                                            {"Prerequisites", json::array({wrongPrerequisite})}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing Prerequisite.Version in response");
            }
        }

        SECTION("Wrong types")
        {
            SECTION("UpdateId")
            {
                const json versionEntity = {{"ContentId", contentId},
                                            {"UpdateId", 1},
                                            {"Prerequisites", json::array({contentId})}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "UpdateId is not a string");
            }

            SECTION("Prerequisites")
            {
                SECTION("Not an array")
                {
                    const json versionEntity = {{"ContentId", contentId},
                                                {"UpdateId", updateId},
                                                {"Prerequisites", contentId}};
                    REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisites is not an array");
                }

                SECTION("Element is not an object")
                {
                    const json versionEntity = {{"ContentId", contentId},
                                                {"UpdateId", updateId},
                                                {"Prerequisites", json::array({1})}};
                    REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisite element is not a JSON object");
                }

                SECTION("Prerequisite.Namespace")
                {
                    const json wrongPrerequisite = {{"Namespace", 1}, {"Name", name}, {"Version", version}};
                    const json versionEntity = {{"ContentId", contentId},
                                                {"UpdateId", updateId},
                                                {"Prerequisites", json::array({wrongPrerequisite})}};
                    REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisite.Namespace is not a string");
                }

                SECTION("Prerequisite.Name")
                {
                    const json wrongPrerequisite = {{"Namespace", ns}, {"Name", 1}, {"Version", version}};
                    const json versionEntity = {{"ContentId", contentId},
                                                {"UpdateId", updateId},
                                                {"Prerequisites", json::array({wrongPrerequisite})}};
                    REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisite.Name is not a string");
                }

                SECTION("Prerequisite.Version")
                {
                    const json wrongPrerequisite = {{"Namespace", ns}, {"Name", name}, {"Version", 1}};
                    const json versionEntity = {{"ContentId", contentId},
                                                {"UpdateId", updateId},
                                                {"Prerequisites", json::array({wrongPrerequisite})}};
                    REQUIRE_THROWS_CODE_MSG(ParseJsonToVersionEntity(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisite.Version is not a string");
                }
            }
        }
    }
}

TEST("Testing ParseJsonToFileEntity()")
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

            REQUIRE_NOTHROW(entity = ParseJsonToFileEntity(fileEntity, handler));
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
                REQUIRE_THROWS_CODE_MSG(ParseJsonToFileEntity(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing File.FileId in response");
            }

            SECTION("Missing Url")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToFileEntity(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing File.Url in response");
            }

            SECTION("Missing SizeInBytes")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToFileEntity(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing File.SizeInBytes in response");
            }

            SECTION("Missing Hashes")
            {
                const json fileEntity = {{"FileId", fileId}, {"Url", url}, {"SizeInBytes", size}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToFileEntity(fileEntity, handler),
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
                REQUIRE_THROWS_CODE_MSG(ParseJsonToFileEntity(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.FileId is not a string");
            }

            SECTION("Url")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", 1},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToFileEntity(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.Url is not a string");
            }

            SECTION("SizeInBytes")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"SizeInBytes", "size"},
                                         {"Hashes", {{"Sha1", sha1}, {"Sha256", sha256}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToFileEntity(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.SizeInBytes is not an unsigned number");
            }

            SECTION("Hashes not an object")
            {
                const json fileEntity = {{"FileId", fileId}, {"Url", url}, {"SizeInBytes", size}, {"Hashes", 1}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToFileEntity(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.Hashes is not an object");
            }

            SECTION("Hashes.Sha1")
            {
                const json fileEntity = {{"FileId", fileId},
                                         {"Url", url},
                                         {"SizeInBytes", size},
                                         {"Hashes", {{"Sha1", 1}}}};
                REQUIRE_THROWS_CODE_MSG(ParseJsonToFileEntity(fileEntity, handler),
                                        ServiceInvalidResponse,
                                        "File.Hashes object value is not a string");
            }
        }
    }
}
