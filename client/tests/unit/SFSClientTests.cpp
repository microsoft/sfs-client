// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/ApplicabilityDetails.h"
#include "sfsclient/DeliveryOptimizationData.h"
#include "sfsclient/SFSClient.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[SFSClientTests] " __VA_ARGS__)
#define TEST_SCENARIO(...) TEST_CASE("[SFSClientTests] Scenario: " __VA_ARGS__)

using namespace SFS;

namespace
{
std::unique_ptr<SFSClient> GetSFSClient()
{
    std::unique_ptr<SFSClient> sfsClient;
    REQUIRE(SFSClient::Make({"testAccountId"}, sfsClient) == Result::S_Ok);
    REQUIRE(sfsClient != nullptr);
    return sfsClient;
}

void TestLoggingCallback(const LogData&)
{
}

struct TestLoggingCallbackStruct
{
    static void TestLoggingCallback(const LogData&)
    {
    }
};
} // namespace

static void StaticTestLoggingCallback(const LogData&)
{
}

TEST("Testing SFSClient::Make()")
{
    const std::string accountId{"testAccountId"};
    const std::string instanceId{"testInstanceId"};
    const std::string nameSpace{"testNameSpace"};

    std::unique_ptr<SFSClient> sfsClient;

    SECTION("Make({accountId}, out)")
    {
        REQUIRE(SFSClient::Make({accountId}, sfsClient) == Result::S_Ok);
        REQUIRE(sfsClient != nullptr);

        Options options{accountId};
        REQUIRE(SFSClient::Make(options, sfsClient) == Result::S_Ok);
        REQUIRE(sfsClient != nullptr);
    }

    SECTION("Make(accountId, instanceId, out)")
    {
        REQUIRE(SFSClient::Make({accountId, instanceId}, sfsClient) == Result::S_Ok);
        REQUIRE(sfsClient != nullptr);

        Options options{accountId, instanceId};
        REQUIRE(SFSClient::Make(options, sfsClient) == Result::S_Ok);
        REQUIRE(sfsClient != nullptr);
    }

    SECTION("Make(accountId, instanceId, namespace, out)")
    {
        REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace}, sfsClient) == Result::S_Ok);
        REQUIRE(sfsClient != nullptr);

        SECTION("Call make when the pointer is reset")
        {
            sfsClient.reset();
            REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace}, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Call make if the pointer is not reset, as Make() resets it")
        {
            REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace}, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("We can also use a separate Options object")
        {
            Options options{accountId, instanceId, nameSpace};
            REQUIRE(SFSClient::Make(options, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("We can also move a separate Options object")
        {
            Options options{accountId, instanceId, nameSpace};
            REQUIRE(SFSClient::Make(std::move(options), sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }
    }

    SECTION("Make(accountId, std::nullopt, nameSpace, out)")
    {
        REQUIRE(SFSClient::Make({accountId, std::nullopt, nameSpace}, sfsClient) == Result::S_Ok);
        REQUIRE(sfsClient != nullptr);

        Options options;
        options.accountId = accountId;
        options.nameSpace = nameSpace;
        REQUIRE(SFSClient::Make(options, sfsClient) == Result::S_Ok);
        REQUIRE(sfsClient != nullptr);
    }

    SECTION("Make(accountId, instanceId, namespace, logCallbackFn, out) works")
    {
        SECTION("Using a lambda with {} initialization")
        {
            REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace, [](const LogData&) {}}, sfsClient) ==
                    Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a lambda with an Options object")
        {
            Options options{accountId, instanceId, nameSpace, [](const LogData&) {}};
            REQUIRE(SFSClient::Make(options, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a nullptr with an Options object")
        {
            Options options{accountId, instanceId, nameSpace, nullptr};
            REQUIRE(SFSClient::Make(options, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a valid empty-namespace function within an Options object")
        {
            Options options{accountId, instanceId, nameSpace, TestLoggingCallback};
            REQUIRE(SFSClient::Make(options, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a valid static function within an Options object")
        {
            Options options{accountId, instanceId, nameSpace, StaticTestLoggingCallback};
            REQUIRE(SFSClient::Make(options, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a valid static member method within an Options object")
        {
            Options options{accountId, instanceId, nameSpace, &TestLoggingCallbackStruct::TestLoggingCallback};
            REQUIRE(SFSClient::Make(options, sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Can also move a lambda")
        {
            Options options{accountId, instanceId, nameSpace, [](const LogData&) {}};
            REQUIRE(SFSClient::Make(std::move(options), sfsClient) == Result::S_Ok);
            REQUIRE(sfsClient != nullptr);
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
