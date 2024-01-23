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
    SFSClientImpl<CurlConnectionManager> sfsClient("testAccountId", "testInstanceId", "testNameSpace");
    auto connection = sfsClient.GetConnectionManager().MakeConnection();

    SECTION("Testing SFSClientImpl::GetLatestVersion()")
    {
        std::unique_ptr<VersionResponse> response;
        REQUIRE(sfsClient.GetLatestVersion("productName", std::nullopt, *connection, response) == Result::E_NotImpl);

        const SearchAttributes attributes{{"attr1", "value"}};
        REQUIRE(sfsClient.GetLatestVersion("productName", attributes, *connection, response) == Result::E_NotImpl);
    }

    SECTION("Testing SFSClientImpl::GetSpecificVersion()")
    {
        std::unique_ptr<VersionResponse> response;
        REQUIRE(sfsClient.GetSpecificVersion("productName", "version", std::nullopt, *connection, response) ==
                Result::E_NotImpl);

        const SearchAttributes attributes{{"attr1", "value"}};
        REQUIRE(sfsClient.GetSpecificVersion("productName", "version", attributes, *connection, response) ==
                Result::E_NotImpl);
    }

    SECTION("Testing SFSClientImpl::GetDownloadInfo()")
    {
        std::unique_ptr<DownloadInfoResponse> response;
        REQUIRE(sfsClient.GetDownloadInfo("productName", "version", std::nullopt, *connection, response) ==
                Result::E_NotImpl);

        const SearchAttributes attributes{{"attr1", "value"}};
        REQUIRE(sfsClient.GetDownloadInfo("productName", "version", attributes, *connection, response) ==
                Result::E_NotImpl);
    }
}

TEST("Testing SFSClientImpl::SetCustomBaseUrl()")
{
    SFSClientImpl<MockConnectionManager> sfsClient("testAccountId", "testInstanceId", "testNameSpace");

    REQUIRE(sfsClient.GetBaseUrl() == "https://testAccountId.api.cdp.microsoft.com");

    sfsClient.SetCustomBaseUrl("customUrl");
    REQUIRE(sfsClient.GetBaseUrl() == "customUrl");
}

TEST("Testing SFSClientImpl::SetLoggingCallback()")
{
    SFSClientImpl<MockConnectionManager> sfsClient("testAccountId", "testInstanceId", "testNameSpace");

    sfsClient.SetLoggingCallback([](const LogData&) {});
    sfsClient.SetLoggingCallback(nullptr);
}
