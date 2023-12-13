// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "details/Responses.h"
#include "details/SFSClientImpl.h"

#include <gtest/gtest.h>

using namespace SFS;
using namespace SFS::details;

TEST(SFSClientImplTests, GetLatestVersion)
{
    SFSClientImpl sfsClient("testAccountId", "testInstanceId", "testNameSpace");

    std::unique_ptr<VersionResponse> response;
    ASSERT_EQ(sfsClient.GetLatestVersion("productName", std::nullopt, response).GetCode(), Result::E_NotImpl);

    const SearchAttributes attributes{{"attr1", "value"}};
    ASSERT_EQ(sfsClient.GetLatestVersion("productName", attributes, response).GetCode(), Result::E_NotImpl);
}

TEST(SFSClientImplTests, GetSpecificVersion)
{
    SFSClientImpl sfsClient("testAccountId", "testInstanceId", "testNameSpace");

    std::unique_ptr<VersionResponse> response;
    ASSERT_EQ(sfsClient.GetSpecificVersion("productName", "version", std::nullopt, response).GetCode(),
              Result::E_NotImpl);

    const SearchAttributes attributes{{"attr1", "value"}};
    ASSERT_EQ(sfsClient.GetSpecificVersion("productName", "version", attributes, response).GetCode(),
              Result::E_NotImpl);
}

TEST(SFSClientImplTests, GetDownloadInfo)
{
    SFSClientImpl sfsClient("testAccountId", "testInstanceId", "testNameSpace");

    std::unique_ptr<DownloadInfoResponse> response;
    ASSERT_EQ(sfsClient.GetDownloadInfo("productName", "version", std::nullopt, response).GetCode(), Result::E_NotImpl);

    const SearchAttributes attributes{{"attr1", "value"}};
    ASSERT_EQ(sfsClient.GetDownloadInfo("productName", "version", attributes, response).GetCode(), Result::E_NotImpl);
}

TEST(SFSClientImplTests, BuildUrl)
{
    // TODO
}
