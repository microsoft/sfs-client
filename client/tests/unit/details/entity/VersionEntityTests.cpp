// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../../util/SFSExceptionMatcher.h"
#include "../../../util/TestHelper.h"
#include "ReportingHandler.h"
#include "entity/VersionEntity.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#define TEST(...) TEST_CASE("[VersionEntityTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;
using namespace SFS::test;
using json = nlohmann::json;

TEST("Testing VersionEntity::FromJson()")
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

            REQUIRE_NOTHROW(entity = VersionEntity::FromJson(versionEntity, handler));
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
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing ContentId in response");
            }

            SECTION("Missing ContentId.Namespace")
            {
                const json versionEntity = {{"ContentId", {{"Name", name}, {"Version", version}}}};
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing ContentId.Namespace in response");
            }

            SECTION("Missing ContentId.Name")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", ns}, {"Version", version}}}};
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing ContentId.Name in response");
            }

            SECTION("Missing ContentId.Version")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", ns}, {"Name", name}}}};
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing ContentId.Version in response");
            }
        }

        SECTION("Wrong types")
        {
            SECTION("ContentId not an object")
            {
                const json versionEntity = json::array({{"Namespace", 1}, {"Name", name}, {"Version", version}});
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Response is not a JSON object");
            }

            SECTION("ContentId.Namespace")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", 1}, {"Name", name}, {"Version", version}}}};
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "ContentId.Namespace is not a string");
            }

            SECTION("ContentId.Name")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", ns}, {"Name", 1}, {"Version", version}}}};
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "ContentId.Name is not a string");
            }

            SECTION("ContentId.Version")
            {
                const json versionEntity = {{"ContentId", {{"Namespace", ns}, {"Name", name}, {"Version", 1}}}};
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
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

            REQUIRE_NOTHROW(entity = VersionEntity::FromJson(versionEntity, handler));
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
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing Prerequisites in response");
            }

            SECTION("Missing Prerequisite.Namespace")
            {
                const json wrongPrerequisite = {{"Name", name}, {"Version", version}};
                const json versionEntity = {{"ContentId", contentId},
                                            {"UpdateId", updateId},
                                            {"Prerequisites", json::array({wrongPrerequisite})}};
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing Prerequisite.Namespace in response");
            }

            SECTION("Missing Prerequisite.Name")
            {
                const json wrongPrerequisite = {{"Namespace", ns}, {"Version", version}};
                const json versionEntity = {{"ContentId", contentId},
                                            {"UpdateId", updateId},
                                            {"Prerequisites", json::array({wrongPrerequisite})}};
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                        ServiceInvalidResponse,
                                        "Missing Prerequisite.Name in response");
            }

            SECTION("Missing Prerequisite.Version")
            {
                const json wrongPrerequisite = {{"Namespace", ns}, {"Name", name}};
                const json versionEntity = {{"ContentId", contentId},
                                            {"UpdateId", updateId},
                                            {"Prerequisites", json::array({wrongPrerequisite})}};
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
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
                REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
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
                    REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisites is not an array");
                }

                SECTION("Element is not an object")
                {
                    const json versionEntity = {{"ContentId", contentId},
                                                {"UpdateId", updateId},
                                                {"Prerequisites", json::array({1})}};
                    REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisite element is not a JSON object");
                }

                SECTION("Prerequisite.Namespace")
                {
                    const json wrongPrerequisite = {{"Namespace", 1}, {"Name", name}, {"Version", version}};
                    const json versionEntity = {{"ContentId", contentId},
                                                {"UpdateId", updateId},
                                                {"Prerequisites", json::array({wrongPrerequisite})}};
                    REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisite.Namespace is not a string");
                }

                SECTION("Prerequisite.Name")
                {
                    const json wrongPrerequisite = {{"Namespace", ns}, {"Name", 1}, {"Version", version}};
                    const json versionEntity = {{"ContentId", contentId},
                                                {"UpdateId", updateId},
                                                {"Prerequisites", json::array({wrongPrerequisite})}};
                    REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisite.Name is not a string");
                }

                SECTION("Prerequisite.Version")
                {
                    const json wrongPrerequisite = {{"Namespace", ns}, {"Name", name}, {"Version", 1}};
                    const json versionEntity = {{"ContentId", contentId},
                                                {"UpdateId", updateId},
                                                {"Prerequisites", json::array({wrongPrerequisite})}};
                    REQUIRE_THROWS_CODE_MSG(VersionEntity::FromJson(versionEntity, handler),
                                            ServiceInvalidResponse,
                                            "Prerequisite.Version is not a string");
                }
            }
        }
    }
}
