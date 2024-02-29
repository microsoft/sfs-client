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
    ClientConfig options;
    options.accountId = "testAccountId";
    REQUIRE(SFSClient::Make(options, sfsClient) == Result::Success);
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
#ifdef __GNUG__
// For GCC, explicitly turning off "missing-field-initializers" warning as this block is testing the scenario
// in which a user calls explicitly onto the API with field initializers for ClientConfig
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

    const std::string accountId{"testAccountId"};
    const std::string instanceId{"testInstanceId"};
    const std::string nameSpace{"testNameSpace"};

    std::unique_ptr<SFSClient> sfsClient;

    SECTION("Make({accountId}, out)")
    {
        REQUIRE(SFSClient::Make({accountId}, sfsClient) == Result::Success);
        REQUIRE(sfsClient != nullptr);

        ClientConfig config{accountId};
        REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
        REQUIRE(sfsClient != nullptr);
    }

    SECTION("Make(accountId, instanceId, out)")
    {
        REQUIRE(SFSClient::Make({accountId, instanceId}, sfsClient) == Result::Success);
        REQUIRE(sfsClient != nullptr);

        ClientConfig config{accountId, instanceId};
        REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
        REQUIRE(sfsClient != nullptr);
    }

    SECTION("Make(accountId, instanceId, namespace, out)")
    {
        REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace}, sfsClient) == Result::Success);
        REQUIRE(sfsClient != nullptr);

        SECTION("Call make when the pointer is reset")
        {
            sfsClient.reset();
            REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace}, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Call make if the pointer is not reset, as Make() resets it")
        {
            REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace}, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("We can also use a separate ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("We can also move a separate ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace};
            REQUIRE(SFSClient::Make(std::move(config), sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }
    }

    SECTION("Make(accountId, std::nullopt, nameSpace, out)")
    {
        REQUIRE(SFSClient::Make({accountId, std::nullopt, nameSpace}, sfsClient) == Result::Success);
        REQUIRE(sfsClient != nullptr);

        ClientConfig config;
        config.accountId = accountId;
        config.nameSpace = nameSpace;
        REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
        REQUIRE(sfsClient != nullptr);
    }

    SECTION("Make(accountId, instanceId, namespace, connectionConfig, out)")
    {
        SECTION("Using an existing struct")
        {
            const ConnectionConfig connectionConfig;
            REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace, connectionConfig, std::nullopt}, sfsClient) ==
                    Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a separate ClientConfig object")
        {
            const ConnectionConfig connectionConfig;
            ClientConfig config{accountId, instanceId, nameSpace, connectionConfig, std::nullopt};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Editing a ConnectionConfig object")
        {
            ConnectionConfig connectionConfig;
            connectionConfig.maxRetries = 2;

            ClientConfig config{accountId, instanceId, nameSpace, connectionConfig, std::nullopt};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Creating a connectionConfig member directly with {}")
        {
            ClientConfig config{accountId, instanceId, nameSpace, {}, std::nullopt};
            config.connectionConfig = {2, 30000};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Testing validation of ConnectionConfig::maxRetries")
        {
            ClientConfig config{accountId, instanceId, nameSpace, {}, std::nullopt};
            for (unsigned i = 0; i < 10; ++i)
            {
                config.connectionConfig.maxRetries = i;
                if (i <= 3)
                {
                    REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
                }
                else
                {
                    const auto ret = SFSClient::Make(config, sfsClient);
                    REQUIRE(ret.GetCode() == Result::InvalidArg);
                    REQUIRE(ret.GetMessage() == "maxRetries must be <= 3");
                }
            }
        }

        SECTION("Testing validation of ConnectionConfig::retryDelayMs")
        {
            ClientConfig config{accountId, instanceId, nameSpace, {}, std::nullopt};
            for (unsigned i = 0; i < 100000; i = i + 5000)
            {
                config.connectionConfig.retryDelayMs = i;
                if (i >= 15000 && i <= 60000)
                {
                    REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
                }
                else
                {
                    const auto ret = SFSClient::Make(config, sfsClient);
                    REQUIRE(ret.GetCode() == Result::InvalidArg);
                    REQUIRE(ret.GetMessage() == "timeoutMs must be between 15000 and 60000");
                }
            }

            INFO("Testing upper bound of retryDelayMs");
            config.connectionConfig.retryDelayMs = 60001;
            auto ret = SFSClient::Make(config, sfsClient);
            REQUIRE(ret.GetCode() == Result::InvalidArg);
            REQUIRE(ret.GetMessage() == "timeoutMs must be between 15000 and 60000");

            INFO("Testing lower bound of retryDelayMs");
            config.connectionConfig.retryDelayMs = 14999;
            ret = SFSClient::Make(config, sfsClient);
            REQUIRE(ret.GetCode() == Result::InvalidArg);
            REQUIRE(ret.GetMessage() == "timeoutMs must be between 15000 and 60000");
        }
    }

    SECTION("Make(accountId, instanceId, namespace, connectionConfig, logCallbackFn, out) works")
    {
        SECTION("Using a lambda with {} initialization")
        {
            REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace, {}, [](const LogData&) {}}, sfsClient) ==
                    Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a lambda with a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, {}, [](const LogData&) {}};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a nullptr with a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, {}, nullptr};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a valid empty-namespace function within a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, {}, TestLoggingCallback};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a valid static function within a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, {}, StaticTestLoggingCallback};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a valid static member method within a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, {}, &TestLoggingCallbackStruct::TestLoggingCallback};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Can also move a lambda")
        {
            ClientConfig config{accountId, instanceId, nameSpace, {}, [](const LogData&) {}};
            REQUIRE(SFSClient::Make(std::move(config), sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }
    }

    SECTION("AccountId cannot be empty")
    {
        REQUIRE(SFSClient::Make({}, sfsClient) == Result::InvalidArg);
        REQUIRE(sfsClient == nullptr);

        ClientConfig config;
        REQUIRE(SFSClient::Make(config, sfsClient) == Result::InvalidArg);
        REQUIRE(sfsClient == nullptr);

        config = {};
        REQUIRE(SFSClient::Make(config, sfsClient) == Result::InvalidArg);
        REQUIRE(sfsClient == nullptr);

        config.accountId = std::string();
        REQUIRE(SFSClient::Make(config, sfsClient) == Result::InvalidArg);
        REQUIRE(sfsClient == nullptr);

        config.instanceId = instanceId;
        REQUIRE(SFSClient::Make(config, sfsClient) == Result::InvalidArg);
        REQUIRE(sfsClient == nullptr);
    }

#ifdef __GNUG__
// For "-Wmissing-field-initializers"
#pragma GCC diagnostic pop
#endif
}

TEST("Testing SFSClient::GetLatestDownloadInfo()")
{
    auto sfsClient = GetSFSClient();
    std::unique_ptr<Content> content;
    RequestParams params;

    SECTION("Does not allow an empty product")
    {
        params.productRequests = {{"", {}}};
        auto result = sfsClient->GetLatestDownloadInfo(params, content);
        REQUIRE(result.GetCode() == Result::InvalidArg);
        REQUIRE(result.GetMessage() == "product cannot be empty");
        REQUIRE(content == nullptr);

        const TargetingAttributes attributes{{"attr1", "value"}};
        params.productRequests = {{"", attributes}};
        result = sfsClient->GetLatestDownloadInfo(params, content);
        REQUIRE(result.GetCode() == Result::InvalidArg);
        REQUIRE(result.GetMessage() == "product cannot be empty");
        REQUIRE(content == nullptr);
    }

    SECTION("Does not allow an empty request")
    {
        auto result = sfsClient->GetLatestDownloadInfo(params, content);
        REQUIRE(result.GetCode() == Result::InvalidArg);
        REQUIRE(result.GetMessage() == "productRequests cannot be empty");
        REQUIRE(content == nullptr);
    }

    SECTION("Accepting multiple products is not implemented yet")
    {
        params.productRequests = {{"p1", {}}, {"p2", {}}};
        auto result = sfsClient->GetLatestDownloadInfo(params, content);
        REQUIRE(result.GetCode() == Result::NotImpl);
        REQUIRE(result.GetMessage() == "There cannot be more than 1 productRequest at the moment");
        REQUIRE(content == nullptr);
    }

    SECTION("Fails if base cv is not correct")
    {
        params.productRequests = {{"p1", {}}};
        params.baseCV = "";
        auto result = sfsClient->GetLatestDownloadInfo(params, content);
        REQUIRE(result.GetCode() == Result::InvalidArg);
        REQUIRE(result.GetMessage() == "cv must not be empty");
        REQUIRE(content == nullptr);

        params.baseCV = "cv";
        result = sfsClient->GetLatestDownloadInfo(params, content);
        REQUIRE(result.GetCode() == Result::InvalidArg);
        REQUIRE(result.GetMessage().find("baseCV is not a valid correlation vector:") == 0);
        REQUIRE(content == nullptr);
    }
}

TEST_SCENARIO("Testing SFSClient::GetDeliveryOptimizationData()")
{
    GIVEN("An SFSClient")
    {
        auto sfsClient = GetSFSClient();

        THEN("SFSClient::GetDeliveryOptimizationData() is not implemented")
        {
            const TargetingAttributes attributes{{"attr1", "value"}};

            std::unique_ptr<Content> content;
            std::unique_ptr<DeliveryOptimizationData> data;
            REQUIRE(sfsClient->GetDeliveryOptimizationData(*content, data) == Result::NotImpl);
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
            const TargetingAttributes attributes{{"attr1", "value"}};

            std::unique_ptr<Content> content;
            std::unique_ptr<ApplicabilityDetails> details;
            REQUIRE(sfsClient->GetApplicabilityDetails(*content, details) == Result::NotImpl);
        }
    }
}
