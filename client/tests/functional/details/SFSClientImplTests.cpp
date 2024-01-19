// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../mock/MockWebServer.h"
#include "../../util/TestHelper.h"
#include "Responses.h"
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

void CheckDownloadInfo(const nlohmann::json& info, const std::string& name)
{
    REQUIRE(info.is_array());
    REQUIRE(info.size() == 2);
    REQUIRE((info[0].contains("FileId") && info[1].contains("FileId")));
    REQUIRE(info[0]["FileId"].get<std::string>() == (name + ".json"));
    REQUIRE(info[1]["FileId"].get<std::string>() == (name + ".bin"));
}
} // namespace

TEST("Testing class SFSClientImpl()")
{
    test::MockWebServer server;
    const std::string ns = "testNameSpace";
    SFSClientImpl<CurlConnectionManager> sfsClient({"testAccountId", "testInstanceId", ns, LogCallbackToTest});
    sfsClient.SetCustomBaseUrl(server.GetBaseUrl());

    server.RegisterProduct("productName", "0.0.0.2");
    server.RegisterProduct("productName", "0.0.0.1");

    auto connection = sfsClient.GetConnectionManager().MakeConnection();

    SECTION("Testing SFSClientImpl::GetLatestVersion()")
    {
        std::unique_ptr<ContentId> contentId;

        SECTION("No attributes")
        {
            REQUIRE(sfsClient.GetLatestVersion("productName", {}, *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.2");
        }

        SECTION("With attributes")
        {
            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE(sfsClient.GetLatestVersion("productName", attributes, *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.2");
        }

        SECTION("Wrong product name")
        {
            REQUIRE(sfsClient.GetLatestVersion("badName", {}, *connection, contentId) == Result::HttpNotFound);
            REQUIRE(!contentId);

            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE(sfsClient.GetLatestVersion("badName", attributes, *connection, contentId) == Result::HttpNotFound);
            REQUIRE(!contentId);
        }
    }

    SECTION("Testing SFSClientImpl::GetSpecificVersion()")
    {
        std::unique_ptr<ContentId> contentId;

        SECTION("Getting 0.0.0.1")
        {
            REQUIRE(sfsClient.GetSpecificVersion("productName", "0.0.0.1", *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.1");
        }

        SECTION("Getting 0.0.0.2")
        {
            REQUIRE(sfsClient.GetSpecificVersion("productName", "0.0.0.2", *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.2");
        }

        SECTION("Wrong product name")
        {
            REQUIRE(sfsClient.GetSpecificVersion("badName", "0.0.0.2", *connection, contentId) == Result::HttpNotFound);
            REQUIRE(!contentId);
        }

        SECTION("Wrong version")
        {
            REQUIRE(sfsClient.GetSpecificVersion("productName", "0.0.0.3", *connection, contentId) ==
                    Result::HttpNotFound);
            REQUIRE(!contentId);
        }
    }

    SECTION("Testing SFSClientImpl::GetDownloadInfo()")
    {
        SECTION("Getting 0.0.0.1")
        {
            std::unique_ptr<DownloadInfoResponse> response;
            REQUIRE(sfsClient.GetDownloadInfo("productName", "0.0.0.1", *connection, response) == Result::Success);
            CheckDownloadInfo(response->GetResponseData(), "productName");
        }

        SECTION("Getting 0.0.0.2")
        {
            std::unique_ptr<DownloadInfoResponse> response;
            REQUIRE(sfsClient.GetDownloadInfo("productName", "0.0.0.2", *connection, response) == Result::Success);
            CheckDownloadInfo(response->GetResponseData(), "productName");
        }

        SECTION("Wrong product name")
        {
            std::unique_ptr<DownloadInfoResponse> response;
            REQUIRE(sfsClient.GetDownloadInfo("badName", "0.0.0.2", *connection, response) == Result::HttpNotFound);
        }

        SECTION("Wrong version")
        {
            std::unique_ptr<DownloadInfoResponse> response;
            REQUIRE(sfsClient.GetDownloadInfo("productName", "0.0.0.3", *connection, response) == Result::HttpNotFound);
        }
    }

    REQUIRE(server.Stop() == Result::Success);
}
