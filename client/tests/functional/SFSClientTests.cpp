// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../mock/MockWebServer.h"
#include "../util/TestHelper.h"
#include "TestOverride.h"
#include "sfsclient/SFSClient.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[Functional][SFSClientTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::test;

const std::string c_instanceId = "testInstanceId";
const std::string c_namespace = "testNamespace";
const std::string c_productName = "testProduct";
const std::string c_version = "0.0.1";
const std::string c_nextVersion = "0.0.2";

namespace
{
void CheckContentId(const ContentId& contentId, const std::string& version)
{
    REQUIRE(contentId.GetNameSpace() == c_namespace);
    REQUIRE(contentId.GetName() == c_productName);
    REQUIRE(contentId.GetVersion() == version);
}

void CheckFiles(const std::vector<std::unique_ptr<File>>& files)
{
    REQUIRE(files.size() == 2);
    REQUIRE(files[0]->GetFileId() == (c_productName + ".json"));
    REQUIRE(files[0]->GetUrl() == ("http://localhost/1.json"));
    REQUIRE(files[1]->GetFileId() == (c_productName + ".bin"));
    REQUIRE(files[1]->GetUrl() == ("http://localhost/2.bin"));
}

void CheckMockContent(const Content& content, const std::string& version)
{
    CheckContentId(content.GetContentId(), version);
    CheckFiles(content.GetFiles());
}
} // namespace

TEST("Testing SFSClient::GetLatestDownloadInfo()")
{
    if (!AreTestOverridesAllowed())
    {
        return;
    }

    test::MockWebServer server;
    std::unique_ptr<ScopedTestOverride> testOverride;
    REQUIRE(ScopedTestOverride::Make(TestOverride::BaseUrl, server.GetBaseUrl(), testOverride));

    std::unique_ptr<SFSClient> sfsClient;
    REQUIRE(SFSClient::Make({"testAccountId", c_instanceId, c_namespace, LogCallbackToTest}, sfsClient) ==
            Result::Success);
    REQUIRE(sfsClient != nullptr);

    server.RegisterProduct(c_productName, c_version);

    std::unique_ptr<Content> content;

    SECTION("No attributes")
    {
        REQUIRE(sfsClient->GetLatestDownloadInfo(c_productName, content) == Result::Success);
        REQUIRE(content);
        CheckMockContent(*content, c_version);
    }

    SECTION("With attributes")
    {
        const SearchAttributes attributes{{"attr1", "value"}};
        REQUIRE(sfsClient->GetLatestDownloadInfo(c_productName, attributes, content) == Result::Success);
        REQUIRE(content);
        CheckMockContent(*content, c_version);
    }

    SECTION("Wrong product name")
    {
        REQUIRE(sfsClient->GetLatestDownloadInfo("badName", {}, content) == Result::HttpNotFound);
        REQUIRE(!content);

        const SearchAttributes attributes{{"attr1", "value"}};
        REQUIRE(sfsClient->GetLatestDownloadInfo("badName", attributes, content) == Result::HttpNotFound);
        REQUIRE(!content);
    }

    SECTION("Adding new version")
    {
        server.RegisterProduct(c_productName, c_nextVersion);

        REQUIRE(sfsClient->GetLatestDownloadInfo(c_productName, content) == Result::Success);
        REQUIRE(content);
        CheckMockContent(*content, c_nextVersion);
    }
}
