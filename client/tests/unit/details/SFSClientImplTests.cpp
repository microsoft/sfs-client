// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Responses.h"
#include "SFSClientImpl.h"
#include "connection/Connection.h"
#include "connection/ConnectionManager.h"
#include "connection/CurlConnectionManager.h"
#include "connection/mock/MockConnectionManager.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[SFSClientImplTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;

TEST("Testing class SFSClientImpl()")
{
    SFSClientImpl<CurlConnectionManager> sfsClient({"testAccountId", "testInstanceId", "testNameSpace", std::nullopt});
    auto connection = sfsClient.GetConnectionManager().MakeConnection();

    SECTION("Testing SFSClientImpl::GetLatestVersion()")
    {
        std::unique_ptr<VersionResponse> response;
        REQUIRE(sfsClient.GetLatestVersion("productName", std::nullopt, *connection, response) == Result::NotImpl);

        const SearchAttributes attributes{{"attr1", "value"}};
        REQUIRE(sfsClient.GetLatestVersion("productName", attributes, *connection, response) == Result::NotImpl);
    }

    SECTION("Testing SFSClientImpl::GetSpecificVersion()")
    {
        std::unique_ptr<VersionResponse> response;
        REQUIRE(sfsClient.GetSpecificVersion("productName", "version", *connection, response) == Result::NotImpl);
    }

    SECTION("Testing SFSClientImpl::GetDownloadInfo()")
    {
        std::unique_ptr<DownloadInfoResponse> response;
        REQUIRE(sfsClient.GetDownloadInfo("productName", "version", *connection, response) == Result::NotImpl);
    }
}

TEST("Testing SFSClientImpl::SetCustomBaseUrl()")
{
    SFSClientImpl<MockConnectionManager> sfsClient({"testAccountId", "testInstanceId", "testNameSpace", std::nullopt});

    REQUIRE(sfsClient.GetBaseUrl() == "https://testAccountId.api.cdp.microsoft.com");

    sfsClient.SetCustomBaseUrl("customUrl");
    REQUIRE(sfsClient.GetBaseUrl() == "customUrl");
}

TEST("Testing passing a logging callback to constructor of SFSClientImpl")
{
    SFSClientImpl<MockConnectionManager> sfsClient(
        {"testAccountId", "testInstanceId", "testNameSpace", [](const LogData&) {}});
    SFSClientImpl<MockConnectionManager> sfsClient2({"testAccountId", "testInstanceId", "testNameSpace", nullptr});
}
