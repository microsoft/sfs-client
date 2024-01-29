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
void CheckProduct(const nlohmann::json& product, std::string_view name, std::string_view version)
{
    REQUIRE(product.is_object());
    REQUIRE((product.contains("ContentId") && product.contains("Files")));
    REQUIRE((product["ContentId"].contains("Namespace") && product["ContentId"].contains("Name") &&
             product["ContentId"].contains("Version")));
    REQUIRE(product["Files"].is_array());
    REQUIRE(product["ContentId"]["Name"].get<std::string>() == name);
    REQUIRE(product["ContentId"]["Version"].get<std::string>() == version);
}

void CheckProductArray(const nlohmann::json& productArray, std::string_view name, std::string_view version)
{
    REQUIRE(productArray.is_array());
    REQUIRE(productArray.size() == 1);

    const auto& product = productArray[0];
    REQUIRE((product.contains("ContentId")));
    REQUIRE((product["ContentId"].contains("Namespace") && product["ContentId"].contains("Name") &&
             product["ContentId"].contains("Version")));
    REQUIRE(product["ContentId"]["Name"].get<std::string>() == name);
    REQUIRE(product["ContentId"]["Version"].get<std::string>() == version);
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
    SFSClientImpl<CurlConnectionManager> sfsClient(
        {"testAccountId", "testInstanceId", "testNameSpace", LogCallbackToTest});
    sfsClient.SetCustomBaseUrl(server.GetBaseUrl());

    server.RegisterProduct("productName", "0.0.0.2");
    server.RegisterProduct("productName", "0.0.0.1");

    auto connection = sfsClient.GetConnectionManager().MakeConnection();

    SECTION("Testing SFSClientImpl::GetLatestVersion()")
    {
        SECTION("No attributes")
        {
            std::unique_ptr<VersionResponse> response;
            REQUIRE(sfsClient.GetLatestVersion("productName", std::nullopt, *connection, response) == Result::Success);
            CheckProductArray(response->GetResponseData(), "productName", "0.0.0.2");
        }

        SECTION("With attributes")
        {
            std::unique_ptr<VersionResponse> response;
            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE(sfsClient.GetLatestVersion("productName", attributes, *connection, response) == Result::Success);
            CheckProductArray(response->GetResponseData(), "productName", "0.0.0.2");
        }

        SECTION("Wrong product name")
        {
            std::unique_ptr<VersionResponse> response;
            REQUIRE(sfsClient.GetLatestVersion("badName", std::nullopt, *connection, response) == Result::HttpNotFound);

            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE(sfsClient.GetLatestVersion("badName", attributes, *connection, response) == Result::HttpNotFound);
        }
    }

    SECTION("Testing SFSClientImpl::GetSpecificVersion()")
    {
        SECTION("Getting 0.0.0.1")
        {
            std::unique_ptr<VersionResponse> response;
            REQUIRE(sfsClient.GetSpecificVersion("productName", "0.0.0.1", *connection, response) == Result::Success);
            CheckProduct(response->GetResponseData(), "productName", "0.0.0.1");
        }

        SECTION("Getting 0.0.0.2")
        {
            std::unique_ptr<VersionResponse> response;
            REQUIRE(sfsClient.GetSpecificVersion("productName", "0.0.0.2", *connection, response) == Result::Success);
            CheckProduct(response->GetResponseData(), "productName", "0.0.0.2");
        }

        SECTION("Wrong product name")
        {
            std::unique_ptr<VersionResponse> response;
            REQUIRE(sfsClient.GetSpecificVersion("badName", "0.0.0.2", *connection, response) == Result::HttpNotFound);
        }

        SECTION("Wrong version")
        {
            std::unique_ptr<VersionResponse> response;
            REQUIRE(sfsClient.GetSpecificVersion("productName", "0.0.0.3", *connection, response) ==
                    Result::HttpNotFound);
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
