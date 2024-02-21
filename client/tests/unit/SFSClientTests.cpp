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

    SECTION("Make(accountId, instanceId, namespace, logCallbackFn, out) works")
    {
        SECTION("Using a lambda with {} initialization")
        {
            REQUIRE(SFSClient::Make({accountId, instanceId, nameSpace, [](const LogData&) {}}, sfsClient) ==
                    Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a lambda with a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, [](const LogData&) {}};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a nullptr with a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, nullptr};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a valid empty-namespace function within a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, TestLoggingCallback};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a valid static function within a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, StaticTestLoggingCallback};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Using a valid static member method within a ClientConfig object")
        {
            ClientConfig config{accountId, instanceId, nameSpace, &TestLoggingCallbackStruct::TestLoggingCallback};
            REQUIRE(SFSClient::Make(config, sfsClient) == Result::Success);
            REQUIRE(sfsClient != nullptr);
        }

        SECTION("Can also move a lambda")
        {
            ClientConfig config{accountId, instanceId, nameSpace, [](const LogData&) {}};
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

    SECTION("SFSClient::GetLatestDownloadInfo() does not allow an empty product")
    {
        std::unique_ptr<Content> content;
        auto result = sfsClient->GetLatestDownloadInfo("", content);
        REQUIRE(result.GetCode() == Result::InvalidArg);
        REQUIRE(result.GetMessage() == "productName cannot be empty");
        REQUIRE(content == nullptr);

        result = sfsClient->GetLatestDownloadInfo("", {}, content);
        REQUIRE(result.GetCode() == Result::InvalidArg);
        REQUIRE(result.GetMessage() == "productName cannot be empty");
        REQUIRE(content == nullptr);

        const SearchAttributes attributes{{"attr1", "value"}};
        result = sfsClient->GetLatestDownloadInfo("", attributes, content);
        REQUIRE(result.GetCode() == Result::InvalidArg);
        REQUIRE(result.GetMessage() == "productName cannot be empty");
        REQUIRE(content == nullptr);
    }
}

TEST_SCENARIO("Testing SFSClient::GetDeliveryOptimizationData()")
{
    GIVEN("An SFSClient")
    {
        auto sfsClient = GetSFSClient();

        THEN("SFSClient::GetDeliveryOptimizationData() returns NotSet for non-registered values")
        {
            const SearchAttributes attributes{{"attr1", "value"}};

            std::unique_ptr<ContentId> contentId;
            REQUIRE(ContentId::Make({}, {}, {}, contentId));
            std::unique_ptr<File> file;
            REQUIRE(File::Make({}, {}, 0, {}, file));
            std::unique_ptr<DeliveryOptimizationData> data;
            REQUIRE(sfsClient->GetDeliveryOptimizationData(*contentId, *file, data) == Result::NotSet);
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

            std::unique_ptr<Content> content;
            std::unique_ptr<ApplicabilityDetails> details;
            REQUIRE(sfsClient->GetApplicabilityDetails(*content, details) == Result::NotImpl);
        }
    }
}
