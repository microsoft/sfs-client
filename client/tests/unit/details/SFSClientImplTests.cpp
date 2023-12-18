// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Responses.h"
#include "SFSClientImpl.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[SFSClientImplTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;

TEST("Testing class SFSClientImpl()")
{
    SFSClientImpl sfsClient("testAccountId", "testInstanceId", "testNameSpace");

    SECTION("Testing SFSClientImpl::GetLatestVersion()")
    {
        std::unique_ptr<VersionResponse> response;
        REQUIRE(sfsClient.GetLatestVersion("productName", std::nullopt, response) == Result::E_NotImpl);

        const SearchAttributes attributes{{"attr1", "value"}};
        REQUIRE(sfsClient.GetLatestVersion("productName", attributes, response) == Result::E_NotImpl);
    }

    SECTION("Testing SFSClientImpl::GetSpecificVersion()")
    {
        std::unique_ptr<VersionResponse> response;
        REQUIRE(sfsClient.GetSpecificVersion("productName", "version", std::nullopt, response) == Result::E_NotImpl);

        const SearchAttributes attributes{{"attr1", "value"}};
        REQUIRE(sfsClient.GetSpecificVersion("productName", "version", attributes, response) == Result::E_NotImpl);
    }

    SECTION("Testing SFSClientImpl::GetDownloadInfo()")
    {
        std::unique_ptr<DownloadInfoResponse> response;
        REQUIRE(sfsClient.GetDownloadInfo("productName", "version", std::nullopt, response) == Result::E_NotImpl);

        const SearchAttributes attributes{{"attr1", "value"}};
        REQUIRE(sfsClient.GetDownloadInfo("productName", "version", attributes, response) == Result::E_NotImpl);
    }

    SECTION("Testing SFSClientImpl::BuildUrl()")
    {
        // TODO
    }
}

TEST("Testing SFSClientImpl::SetLoggingCallback()")
{
    SFSClientImpl sfsClient("testAccountId", "testInstanceId", "testNameSpace");

    sfsClient.SetLoggingCallback([](const LogData&) {});
    sfsClient.SetLoggingCallback(nullptr);
}
