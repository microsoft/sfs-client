// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/ApplicabilityDetails.h"
#include "sfsclient/DeliveryOptimizationData.h"
#include "sfsclient/SFSClient.h"

#include <gtest/gtest.h>

using namespace SFS;

namespace
{
void GetSFSClient(std::unique_ptr<SFSClient>& sfsClient)
{
    sfsClient.reset();
    ASSERT_EQ(SFSClient::Make("testAccountId", sfsClient).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, sfsClient);
}

std::unique_ptr<SFSClient> GetSFSClient()
{
    std::unique_ptr<SFSClient> sfsClient;
    GetSFSClient(sfsClient);
    return sfsClient;
}
} // namespace

TEST(SFSClientTests, Make)
{
    const std::string accountId{"testAccountId"};
    const std::string instanceId{"testInstanceId"};
    const std::string nameSpace{"testNameSpace"};

    // Testing each of the 3 Make methods
    std::unique_ptr<SFSClient> sfsClient;

    ASSERT_EQ(SFSClient::Make(accountId, sfsClient).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, sfsClient);

    sfsClient.reset();
    ASSERT_EQ(SFSClient::Make(accountId, instanceId, sfsClient).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, sfsClient);

    sfsClient.reset();
    ASSERT_EQ(SFSClient::Make(accountId, instanceId, nameSpace, sfsClient).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, sfsClient);
}

TEST(SFSClientTests, GetLatestDownloadInfo)
{
    auto sfsClient = GetSFSClient();

    ASSERT_EQ(SFSClient::Make("testAccountId", sfsClient).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, sfsClient);

    Contents contents;
    ASSERT_EQ(sfsClient->GetLatestDownloadInfo("productName", contents).GetCode(), Result::E_NotImpl);
}

TEST(SFSClientTests, GetLatestDownloadInfoWithAttributes)
{
    auto sfsClient = GetSFSClient();

    ASSERT_EQ(SFSClient::Make("testAccountId", sfsClient).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, sfsClient);

    const SearchAttributes attributes{{"attr1", "value"}};

    Contents contents;
    ASSERT_EQ(sfsClient->GetLatestDownloadInfo("productName", attributes, contents).GetCode(), Result::E_NotImpl);
}

TEST(SFSClientTests, GetDeliveryOptimizationData)
{
    auto sfsClient = GetSFSClient();

    ASSERT_EQ(SFSClient::Make("testAccountId", sfsClient).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, sfsClient);

    Contents contents;
    ASSERT_EQ(sfsClient->GetLatestDownloadInfo("productName", contents).GetCode(), Result::E_NotImpl);

    std::unique_ptr<Content> content;
    std::unique_ptr<DeliveryOptimizationData> data;
    ASSERT_EQ(sfsClient->GetDeliveryOptimizationData(*content, data).GetCode(), Result::E_NotImpl);
}

TEST(SFSClientTests, GetApplicabilityDetails)
{
    auto sfsClient = GetSFSClient();

    ASSERT_EQ(SFSClient::Make("testAccountId", sfsClient).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, sfsClient);

    Contents contents;
    ASSERT_EQ(sfsClient->GetLatestDownloadInfo("productName", contents).GetCode(), Result::E_NotImpl);

    std::unique_ptr<Content> content;
    std::unique_ptr<ApplicabilityDetails> details;
    ASSERT_EQ(sfsClient->GetApplicabilityDetails(*content, details).GetCode(), Result::E_NotImpl);
}
