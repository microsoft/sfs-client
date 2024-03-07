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
            REQUIRE(entity->contentId.nameSpace == "namespace");
            REQUIRE(entity->contentId.name == "name");
            REQUIRE(entity->contentId.version == "version");
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
}
