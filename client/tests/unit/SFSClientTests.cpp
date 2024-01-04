// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/ApplicabilityDetails.h"
#include "sfsclient/DeliveryOptimizationData.h"
#include "sfsclient/SFSClient.h"

#include <catch2/catch_test_macros.hpp>

#define TEST_SCENARIO(...) TEST_CASE("[SFSClientTests] Scenario: " __VA_ARGS__)

using namespace SFS;

namespace
{
std::unique_ptr<SFSClient> GetSFSClient()
{
    std::unique_ptr<SFSClient> sfsClient;
    REQUIRE(SFSClient::Make("testAccountId", sfsClient) == Result::S_Ok);
    REQUIRE(sfsClient != nullptr);
    return sfsClient;
}
} // namespace

TEST_SCENARIO("Testing SFSClient::Make()")
{
    GIVEN("An uninitialized pointer")
    {
        const std::string accountId{"testAccountId"};
        const std::string instanceId{"testInstanceId"};
        const std::string nameSpace{"testNameSpace"};

        std::unique_ptr<SFSClient> sfsClient;

        THEN("Make(accountId, out) works")
        {
            INFO("bla");
            REQUIRE(SFSClient::Make(accountId, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        THEN("Make(accountId, instanceId, out) works")
        {
            REQUIRE(SFSClient::Make(accountId, instanceId, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        THEN("Make(accountId, instanceId, namespace out) works")
        {
            REQUIRE(SFSClient::Make(accountId, instanceId, nameSpace, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);

            WHEN("The pointer is reset")
            {
                sfsClient.reset();

                THEN("We can call Make a second time")
                {
                    REQUIRE(SFSClient::Make(accountId, instanceId, nameSpace, sfsClient) == Result::S_Ok);
                    REQUIRE(sfsClient != nullptr);
                }
            }

            WHEN("The pointer is not reset")
            {
                THEN("We can also call Make a second time, as Make() resets it")
                {
                    REQUIRE(SFSClient::Make(accountId, instanceId, nameSpace, sfsClient) == Result::S_Ok);
                    REQUIRE(sfsClient != nullptr);
                }
            }
        }
    }
}

TEST_SCENARIO("Testing SFSClient::GetLatestDownloadInfo()")
{
    GIVEN("An SFSClient")
    {
        auto sfsClient = GetSFSClient();

        THEN("SFSClient::GetLatestDownloadInfo() is not implemented")
        {
            Contents contents;
            REQUIRE(sfsClient->GetLatestDownloadInfo("productName", contents) == Result::E_NotImpl);
        }
    }
}

TEST_SCENARIO("Testing SFSClient::GetLatestDownloadInfoWithAttributes()")
{
    GIVEN("An SFSClient")
    {
        auto sfsClient = GetSFSClient();

        THEN("SFSClient::GetLatestDownloadInfo() is not implemented")
        {
            const SearchAttributes attributes{{"attr1", "value"}};

            Contents contents;
            REQUIRE(sfsClient->GetLatestDownloadInfo("productName", attributes, contents) == Result::E_NotImpl);
        }
    }
}

TEST_SCENARIO("Testing SFSClient::GetDeliveryOptimizationData()")
{
    GIVEN("An SFSClient")
    {
        auto sfsClient = GetSFSClient();

        THEN("SFSClient::GetDeliveryOptimizationData() is not implemented")
        {
            const SearchAttributes attributes{{"attr1", "value"}};

            Contents contents;
            REQUIRE(sfsClient->GetLatestDownloadInfo("productName", attributes, contents) == Result::E_NotImpl);

            std::unique_ptr<Content> content;
            std::unique_ptr<DeliveryOptimizationData> data;
            REQUIRE(sfsClient->GetDeliveryOptimizationData(*content, data) == Result::E_NotImpl);
        }
    }
}

TEST_SCENARIO("Testing SFSClient::GetApplicabilityDetails()")
{
    GIVEN("An SFSClient")
    {
        auto sfsClient = GetSFSClient();

        THEN("SFSClient::GetApplicabilityDetails() is not implemented")
        {
            const SearchAttributes attributes{{"attr1", "value"}};

            Contents contents;
            REQUIRE(sfsClient->GetLatestDownloadInfo("productName", attributes, contents) == Result::E_NotImpl);

            std::unique_ptr<Content> content;
            std::unique_ptr<ApplicabilityDetails> details;
            REQUIRE(sfsClient->GetApplicabilityDetails(*content, details) == Result::E_NotImpl);
        }
    }
}
