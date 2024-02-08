// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../mock/MockWebServer.h"
#include "../../util/TestHelper.h"
#include "SFSClientImpl.h"
#include "connection/Connection.h"
#include "connection/CurlConnectionManager.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[Functional][SFSClientImplTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;
using namespace SFS::test;

namespace
{
void CheckProduct(const ContentId& contentId, std::string_view ns, std::string_view name, std::string_view version)
{
    REQUIRE(contentId.GetNameSpace() == ns);
    REQUIRE(contentId.GetName() == name);
    REQUIRE(contentId.GetVersion() == version);
}

void CheckDownloadInfo(const std::vector<std::unique_ptr<File>>& files, const std::string& name)
{
    REQUIRE(files.size() == 2);
    REQUIRE(files[0]->GetFileId() == (name + ".json"));
    REQUIRE(files[0]->GetUrl() == ("http://localhost/1.json"));
    REQUIRE(files[1]->GetFileId() == (name + ".bin"));
    REQUIRE(files[1]->GetUrl() == ("http://localhost/2.bin"));
}
} // namespace

TEST("Testing class SFSClientImpl()")
{
    test::MockWebServer server;
    const std::string ns = "testNameSpace";
    std::unique_ptr<SFSClientInterface> sfsClient;
    REQUIRE(SFSClientImpl<CurlConnectionManager>::Make({"testAccountId", "testInstanceId", ns, LogCallbackToTest},
                                                       sfsClient) == Result::Success);
    dynamic_cast<SFSClientImpl<CurlConnectionManager>*>(sfsClient.get())->SetCustomBaseUrl(server.GetBaseUrl());

    server.RegisterProduct("productName", "0.0.0.2");
    server.RegisterProduct("productName", "0.0.0.1");

    auto connection = sfsClient->GetConnectionManager().MakeConnection();

    SECTION("Testing SFSClientImpl::GetLatestVersion()")
    {
        std::unique_ptr<ContentId> contentId;

        SECTION("No attributes")
        {
            REQUIRE(sfsClient->GetLatestVersion("productName", {}, *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.2");
        }

        SECTION("With attributes")
        {
            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE(sfsClient->GetLatestVersion("productName", attributes, *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.2");
        }

        SECTION("Wrong product name")
        {
            REQUIRE(sfsClient->GetLatestVersion("badName", {}, *connection, contentId) == Result::HttpNotFound);
            REQUIRE(!contentId);

            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE(sfsClient->GetLatestVersion("badName", attributes, *connection, contentId) == Result::HttpNotFound);
            REQUIRE(!contentId);
        }
    }

    SECTION("Testing SFSClientImpl::GetSpecificVersion()")
    {
        std::unique_ptr<ContentId> contentId;

        SECTION("Getting 0.0.0.1")
        {
            REQUIRE(sfsClient->GetSpecificVersion("productName", "0.0.0.1", *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.1");
        }

        SECTION("Getting 0.0.0.2")
        {
            REQUIRE(sfsClient->GetSpecificVersion("productName", "0.0.0.2", *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.2");
        }

        SECTION("Wrong product name")
        {
            REQUIRE(sfsClient->GetSpecificVersion("badName", "0.0.0.2", *connection, contentId) ==
                    Result::HttpNotFound);
            REQUIRE(!contentId);
        }

        SECTION("Wrong version")
        {
            REQUIRE(sfsClient->GetSpecificVersion("productName", "0.0.0.3", *connection, contentId) ==
                    Result::HttpNotFound);
            REQUIRE(!contentId);
        }
    }

    SECTION("Testing SFSClientImpl::GetDownloadInfo()")
    {
        std::vector<std::unique_ptr<File>> files;

        SECTION("Getting 0.0.0.1")
        {
            REQUIRE(sfsClient->GetDownloadInfo("productName", "0.0.0.1", *connection, files) == Result::Success);
            REQUIRE(!files.empty());
            CheckDownloadInfo(files, "productName");
        }

        SECTION("Getting 0.0.0.2")
        {
            REQUIRE(sfsClient->GetDownloadInfo("productName", "0.0.0.2", *connection, files) == Result::Success);
            REQUIRE(!files.empty());
            CheckDownloadInfo(files, "productName");
        }

        SECTION("Wrong product name")
        {
            REQUIRE(sfsClient->GetDownloadInfo("badName", "0.0.0.2", *connection, files) == Result::HttpNotFound);
            REQUIRE(files.empty());
        }

        SECTION("Wrong version")
        {
            REQUIRE(sfsClient->GetDownloadInfo("productName", "0.0.0.3", *connection, files) == Result::HttpNotFound);
            REQUIRE(files.empty());
        }
    }

    REQUIRE(server.Stop() == Result::Success);
}
