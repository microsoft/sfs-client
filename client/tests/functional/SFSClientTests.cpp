// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../mock/MockWebServer.h"
#include "../util/TestHelper.h"
#include "TestOverride.h"
#include "sfsclient/SFSClient.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>

#define TEST(...) TEST_CASE("[Functional][SFSClientTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::test;
using namespace std::chrono;

const std::string c_instanceId = "testInstanceId";
const std::string c_namespace = "testNamespace";
const std::string c_productName = "testProduct";
const std::string c_version = "0.0.1";
const std::string c_nextVersion = "0.0.2";

namespace
{
void CheckContentId(const ContentId& contentId, const std::string& version)
{
    REQUIRE(contentId.GetNameSpace() == c_namespace);
    REQUIRE(contentId.GetName() == c_productName);
    REQUIRE(contentId.GetVersion() == version);
}

void CheckFiles(const std::vector<File>& files)
{
    REQUIRE(files.size() == 2);
    REQUIRE(files[0].GetFileId() == (c_productName + ".json"));
    REQUIRE(files[0].GetUrl() == ("http://localhost/1.json"));
    REQUIRE(files[1].GetFileId() == (c_productName + ".bin"));
    REQUIRE(files[1].GetUrl() == ("http://localhost/2.bin"));
}

void CheckMockContent(const Content& content, const std::string& version)
{
    CheckContentId(content.GetContentId(), version);
    CheckFiles(content.GetFiles());
}
} // namespace

TEST("Testing SFSClient::GetLatestDownloadInfo()")
{
    if (!AreTestOverridesAllowed())
    {
        return;
    }

    test::MockWebServer server;
    ScopedTestOverride override(TestOverride::BaseUrl, server.GetBaseUrl());

    std::unique_ptr<SFSClient> sfsClient;
    REQUIRE(SFSClient::Make({"testAccountId", c_instanceId, c_namespace, {}, LogCallbackToTest}, sfsClient) ==
            Result::Success);
    REQUIRE(sfsClient != nullptr);

    server.RegisterProduct(c_productName, c_version);

    std::unique_ptr<Content> content;

    SECTION("Single product request")
    {
        RequestParams params;
        params.baseCV = "aaaaaaaaaaaaaaaa.1";
        SECTION("No attributes")
        {
            params.productRequests = {{c_productName, {}}};
            REQUIRE(sfsClient->GetLatestDownloadInfo(params, content) == Result::Success);
            REQUIRE(content);
            CheckMockContent(*content, c_version);
        }

        SECTION("With attributes")
        {
            const TargetingAttributes attributes{{"attr1", "value"}};
            params.productRequests = {{c_productName, attributes}};
            REQUIRE(sfsClient->GetLatestDownloadInfo(params, content) == Result::Success);
            REQUIRE(content);
            CheckMockContent(*content, c_version);
        }

        SECTION("Wrong product name")
        {
            params.productRequests = {{"badName", {}}};
            REQUIRE(sfsClient->GetLatestDownloadInfo(params, content) == Result::HttpNotFound);
            REQUIRE(!content);

            const TargetingAttributes attributes{{"attr1", "value"}};
            params.productRequests = {{"badName", attributes}};
            REQUIRE(sfsClient->GetLatestDownloadInfo(params, content) == Result::HttpNotFound);
            REQUIRE(!content);
        }

        SECTION("Adding new version")
        {
            server.RegisterProduct(c_productName, c_nextVersion);

            params.productRequests = {{c_productName, {}}};
            REQUIRE(sfsClient->GetLatestDownloadInfo(params, content) == Result::Success);
            REQUIRE(content);
            CheckMockContent(*content, c_nextVersion);
        }
    }
}

TEST("Testing SFSClient retry behavior")
{
    if (!AreTestOverridesAllowed())
    {
        INFO("Skipping. Test overrides not enabled");
        return;
    }

    MockWebServer server;
    ScopedTestOverride urlOverride(TestOverride::BaseUrl, server.GetBaseUrl());

    server.RegisterProduct(c_productName, c_version);
    RequestParams params;
    params.productRequests = {{c_productName, {}}};
    std::unique_ptr<Content> content;

    std::unique_ptr<SFSClient> sfsClient;
    ClientConfig clientConfig{"testAccountId", c_instanceId, c_namespace, {}, LogCallbackToTest};

    SECTION("Test exponential backoff")
    {
        INFO("Sets the retry delay to 50ms to speed up the test");
        ScopedTestOverride override(TestOverride::BaseRetryDelayMs, 50);

        REQUIRE(SFSClient::Make(clientConfig, sfsClient));
        REQUIRE(sfsClient != nullptr);

        const int retriableError = 503; // ServerBusy

        auto RunTimedGet = [&](bool success = true) -> long long {
            auto begin = steady_clock::now();
            if (success)
            {
                REQUIRE(sfsClient->GetLatestDownloadInfo(params, content));
                REQUIRE(content);
            }
            else
            {
                REQUIRE(sfsClient->GetLatestDownloadInfo(params, content) == Result::HttpServiceNotAvailable);
                REQUIRE_FALSE(content);
            }
            auto end = steady_clock::now();
            return duration_cast<milliseconds>(end - begin).count();
        };

        long long allowedTimeDeviation = 200LL;
        std::queue<HttpCode> forcedHttpErrors({retriableError});
        SECTION("Should take at least 50ms with a single retriable error")
        {
            server.SetForcedHttpErrors(forcedHttpErrors);
            const auto time = RunTimedGet();
            REQUIRE(time >= 50LL);
            REQUIRE(time < 50LL + allowedTimeDeviation);
        }

        forcedHttpErrors.push(retriableError);
        SECTION("Should take at least 150ms (50ms + 2*50ms) with two retriable errors")
        {
            server.SetForcedHttpErrors(forcedHttpErrors);
            const auto time = RunTimedGet();
            REQUIRE(time >= 150LL);
            REQUIRE(time < 150LL + allowedTimeDeviation);
        }

        SECTION("Should fail after first retry if we limit max duration to 75ms")
        {
            clientConfig.connectionConfig.maxRequestDuration =
                std::chrono::duration_cast<std::chrono::minutes>(std::chrono::milliseconds{75});
            REQUIRE(SFSClient::Make(clientConfig, sfsClient));

            server.SetForcedHttpErrors(forcedHttpErrors);
            const auto time = RunTimedGet(false /*success*/);
            REQUIRE(time < 75LL + allowedTimeDeviation);
        }

        forcedHttpErrors.push(retriableError);
        SECTION("Should take at least 300ms (50ms + 2*50ms + 3*50ms) with three retriable errors")
        {
            server.SetForcedHttpErrors(forcedHttpErrors);
            const auto time = RunTimedGet();
            REQUIRE(time >= 300LL);
            REQUIRE(time < 300LL + allowedTimeDeviation);
        }

        SECTION("Should fail after second retry if we limit max duration to 200ms")
        {
            clientConfig.connectionConfig.maxRequestDuration =
                std::chrono::duration_cast<std::chrono::minutes>(std::chrono::milliseconds{200});
            REQUIRE(SFSClient::Make(clientConfig, sfsClient));

            server.SetForcedHttpErrors(forcedHttpErrors);
            const auto time = RunTimedGet(false /*success*/);
            REQUIRE(time < 200LL + allowedTimeDeviation);
        }

        forcedHttpErrors.push(retriableError);
        SECTION("Should take at least 300ms (50ms + 2*50ms + 3*50ms) with four retriable errors, but fail")
        {
            server.SetForcedHttpErrors(forcedHttpErrors);
            const auto time = RunTimedGet(false /*success*/);
            REQUIRE(time >= 300LL);
            REQUIRE(time < 300LL + allowedTimeDeviation);
        }
    }

    SECTION("Test retriable errors with Retry-After headers")
    {
        INFO("Sets the retry delay to 200ms to speed up the test");
        ScopedTestOverride override(TestOverride::BaseRetryDelayMs, 200);

        REQUIRE(SFSClient::Make(clientConfig, sfsClient));
        REQUIRE(sfsClient != nullptr);

        const int retriableError = 503;  // ServerBusy
        const int retriableError2 = 502; // BadGateway

        auto RunTimedGet = [&]() -> long long {
            auto begin = steady_clock::now();
            REQUIRE(sfsClient->GetLatestDownloadInfo(params, content));
            REQUIRE(content);
            auto end = steady_clock::now();
            return duration_cast<milliseconds>(end - begin).count();
        };

        std::unordered_map<HttpCode, HeaderMap> headersByCode;
        headersByCode[retriableError] = {{"Retry-After", "1"}}; // 1s delay
        server.SetResponseHeaders(headersByCode);

        long long allowedTimeDeviation = 200LL;
        std::queue<HttpCode> forcedHttpErrors({retriableError});
        SECTION("Should take at least 1000ms with a single retriable error with 1s in Retry-After")
        {
            server.SetForcedHttpErrors(forcedHttpErrors);
            const auto time = RunTimedGet();
            REQUIRE(time >= 1000LL);
            REQUIRE(time < 1000LL + allowedTimeDeviation);
        }

        SECTION(
            "Should take at least 1000ms + 200ms with a retriable error with 1s in Retry-After and one with 200ms*2 as default value")
        {
            forcedHttpErrors.push(retriableError2);
            server.SetForcedHttpErrors(forcedHttpErrors);
            const auto time = RunTimedGet();
            REQUIRE(time >= 1400LL);
            REQUIRE(time < 1400LL + allowedTimeDeviation);
        }
    }

    SECTION("Test maxRetries")
    {
        INFO("Sets the retry delay to 1ms to speed up the test");
        ScopedTestOverride override(TestOverride::BaseRetryDelayMs, 1);

        const int retriableError = 503; // ServerBusy

        SECTION("With default retries")
        {
            REQUIRE(SFSClient::Make(clientConfig, sfsClient));
            REQUIRE(sfsClient != nullptr);

            SECTION("Should pass with 3 errors")
            {
                server.SetForcedHttpErrors(std::queue<HttpCode>({retriableError, retriableError, retriableError}));
                REQUIRE(sfsClient->GetLatestDownloadInfo(params, content));
            }

            SECTION("Should fail with 4 errors")
            {
                server.SetForcedHttpErrors(
                    std::queue<HttpCode>({retriableError, retriableError, retriableError, retriableError}));
                REQUIRE(sfsClient->GetLatestDownloadInfo(params, content) == Result::HttpServiceNotAvailable);
            }
        }

        SECTION("Reducing retries to 2")
        {
            clientConfig.connectionConfig.maxRetries = 2;
            REQUIRE(SFSClient::Make(clientConfig, sfsClient));
            REQUIRE(sfsClient != nullptr);

            SECTION("Should pass with 2 errors")
            {
                server.SetForcedHttpErrors(std::queue<HttpCode>({retriableError, retriableError}));
                REQUIRE(sfsClient->GetLatestDownloadInfo(params, content));
            }

            SECTION("Should fail with 3 errors")
            {
                server.SetForcedHttpErrors(std::queue<HttpCode>({retriableError, retriableError, retriableError}));
                REQUIRE(sfsClient->GetLatestDownloadInfo(params, content) == Result::HttpServiceNotAvailable);
            }
        }

        SECTION("Reducing retries to 0")
        {
            clientConfig.connectionConfig.maxRetries = 0;
            REQUIRE(SFSClient::Make(clientConfig, sfsClient));
            REQUIRE(sfsClient != nullptr);

            SECTION("Should pass with no errors")
            {
                REQUIRE(sfsClient->GetLatestDownloadInfo(params, content));
            }

            SECTION("Should fail with 1 error")
            {
                server.SetForcedHttpErrors(std::queue<HttpCode>({retriableError}));
                REQUIRE(sfsClient->GetLatestDownloadInfo(params, content) == Result::HttpServiceNotAvailable);
            }
        }
    }
}
