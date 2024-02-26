// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../mock/MockWebServer.h"
#include "../../util/SFSExceptionMatcher.h"
#include "../../util/TestHelper.h"
#include "ReportingHandler.h"
#include "SFSUrlComponents.h"
#include "connection/CurlConnection.h"
#include "connection/CurlConnectionManager.h"
#include "connection/HttpHeader.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#define TEST(...) TEST_CASE("[Functional][CurlConnectionTests] " __VA_ARGS__)

// Define included by Win32 headers
#undef GetMessage

using namespace SFS;
using namespace SFS::details;
using namespace SFS::test;
using json = nlohmann::json;

const std::string c_instanceId = "default";
const std::string c_namespace = "default";
const std::string c_productName = "testProduct";
const std::string c_version = "0.0.1";
const std::string c_nextVersion = "0.0.2";

namespace
{
class CurlConnectionTimeout : public CurlConnection
{
  public:
    CurlConnectionTimeout(const ReportingHandler& handler) : CurlConnection(handler)
    {
    }

    std::string Get(const std::string& url) override
    {
        // Timeout within 100ms
        curl_easy_setopt(m_handle, CURLOPT_TIMEOUT_MS, 100L);
        return CurlConnection::Get(url);
    }

    std::string Post(const std::string& url, const std::string& data) override
    {
        // Timeout within 100ms
        curl_easy_setopt(m_handle, CURLOPT_TIMEOUT_MS, 100L);
        return CurlConnection::Post(url, data);
    }
};

class CurlConnectionTimeoutManager : public CurlConnectionManager
{
  public:
    CurlConnectionTimeoutManager(const ReportingHandler& handler) : CurlConnectionManager(handler)
    {
    }

    std::unique_ptr<Connection> MakeConnection() override
    {
        return std::make_unique<CurlConnectionTimeout>(m_handler);
    }
};
} // namespace

TEST("Testing CurlConnection()")
{
    test::MockWebServer server;
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);
    CurlConnectionManager connectionManager(handler);
    auto connection = connectionManager.MakeConnection();

    SECTION("Testing CurlConnection::Get()")
    {
        const std::string url = SFSUrlComponents::GetSpecificVersionUrl(server.GetBaseUrl(),
                                                                        c_instanceId,
                                                                        c_namespace,
                                                                        c_productName,
                                                                        c_version);

        // Before registering the product, the URL returns 404 Not Found
        REQUIRE_THROWS_CODE(connection->Get(url), HttpNotFound);

        // Register the product
        server.RegisterProduct(c_productName, c_version);

        // After registering the product, the URL returns 200 OK
        std::string out;
        REQUIRE_NOTHROW(out = connection->Get(url));

        json expectedResponse;
        expectedResponse["ContentId"] = {{"Namespace", "default"}, {"Name", c_productName}, {"Version", c_version}};
        expectedResponse["Files"] = json::array({c_productName + ".json", c_productName + ".bin"});

        REQUIRE(json::parse(out) == expectedResponse);
    }

    SECTION("Testing CurlConnection::Post()")
    {
        server.RegisterExpectedRequestHeader(HttpHeader::ContentType, "application/json");

        SECTION("With GetLatestVersionBatch mock")
        {
            const std::string url =
                SFSUrlComponents::GetLatestVersionBatchUrl(server.GetBaseUrl(), c_instanceId, c_namespace);

            // Missing proper body returns HttpBadRequest
            REQUIRE_THROWS_CODE(connection->Post(url), HttpBadRequest);

            // Before registering the product, the URL returns 404 Not Found
            const json body = {{{"TargetingAttributes", {}}, {"Product", c_productName}}};
            REQUIRE_THROWS_CODE(connection->Post(url, body.dump()), HttpNotFound);

            // Register the product
            server.RegisterProduct(c_productName, c_version);

            // After registering the product, the URL returns 200 OK
            std::string out;
            REQUIRE_NOTHROW(out = connection->Post(url, body.dump()));

            json expectedResponse = json::array();
            expectedResponse.push_back(
                {{"ContentId", {{"Namespace", "default"}, {"Name", c_productName}, {"Version", c_version}}}});
            REQUIRE(json::parse(out) == expectedResponse);

            // Returns the next version now
            server.RegisterProduct(c_productName, c_nextVersion);
            REQUIRE_NOTHROW(out = connection->Post(url, body.dump()));

            expectedResponse = json::array();
            expectedResponse.push_back(
                {{"ContentId", {{"Namespace", "default"}, {"Name", c_productName}, {"Version", c_nextVersion}}}});
            REQUIRE(json::parse(out) == expectedResponse);
        }

        SECTION("With GetDownloadInfo mock")
        {
            const std::string url = SFSUrlComponents::GetDownloadInfoUrl(server.GetBaseUrl(),
                                                                         c_instanceId,
                                                                         c_namespace,
                                                                         c_productName,
                                                                         c_version);

            // Before registering the product, the URL returns 404 Not Found
            REQUIRE_THROWS_CODE(connection->Post(url), HttpNotFound);

            // Register the product
            server.RegisterProduct(c_productName, c_version);

            // After registering the product, the URL returns 200 OK
            std::string out;
            REQUIRE_NOTHROW(out = connection->Post(url));

            json expectedResponse = json::array();
            expectedResponse.push_back({{"Url", "http://localhost/1.json"},
                                        {"FileId", c_productName + ".json"},
                                        {"SizeInBytes", 100},
                                        {"Hashes", {{"Sha1", "123"}, {"Sha256", "456"}}}});
            expectedResponse[0]["DeliveryOptimization"] = {{"CatalogId", "789"}};
            expectedResponse[0]["DeliveryOptimization"]["Properties"] = {
                {"IntegrityCheckInfo", {{"PiecesHashFileUrl", "http://localhost/1.json"}, {"HashOfHashes", "abc"}}}};

            expectedResponse.push_back({{"Url", "http://localhost/2.bin"},
                                        {"FileId", c_productName + ".bin"},
                                        {"SizeInBytes", 200},
                                        {"Hashes", {{"Sha1", "421"}, {"Sha256", "132"}}}});
            expectedResponse[1]["DeliveryOptimization"] = {{"CatalogId", "14"}};
            expectedResponse[1]["DeliveryOptimization"]["Properties"] = {
                {"IntegrityCheckInfo", {{"PiecesHashFileUrl", "http://localhost/2.bin"}, {"HashOfHashes", "abcd"}}}};

            REQUIRE(json::parse(out) == expectedResponse);
        }
    }

    SECTION("Testing with a malformed url")
    {
        std::string url = server.GetBaseUrl();
        REQUIRE_THROWS_CODE(connection->Get(url), HttpNotFound);
        REQUIRE_THROWS_CODE(connection->Get(url + "/names"), HttpNotFound);
        REQUIRE_THROWS_CODE(connection->Post(url + "/names", "{}"), HttpNotFound);

        url = server.GetBaseUrl() + "/api/v1/contents/" + c_instanceId + "/namespaces/" + c_namespace + "/names/" +
              c_productName + "/versYions/" + c_version + "/files?action=GenerateDownloadInfo";

        std::string out;
        REQUIRE_THROWS_CODE(connection->Get(url), HttpNotFound);
        REQUIRE_THROWS_CODE(connection->Post(url, std::string()), HttpNotFound);
        REQUIRE_THROWS_CODE(out = connection->Post(url, {}), HttpNotFound);
        CHECK(out.empty());
    }

    REQUIRE(server.Stop() == Result::Success);
}

TEST("Testing CurlConnection when the server is not reachable")
{
    // Using a custom override class just to time out faster on an invalid URL
    ReportingHandler handler;
    CurlConnectionTimeoutManager connectionManager(handler);
    handler.SetLoggingCallback(LogCallbackToTest);
    auto connection = connectionManager.MakeConnection();

    // Using a non-routable IP address to ensure the server is not reachable
    // https://www.rfc-editor.org/rfc/rfc5737#section-3: The blocks 192.0.2.0/24 (...) are provided for use in
    // documentation.
    std::string url = "192.0.2.0";
    std::string out;
    REQUIRE_THROWS_CODE(connection->Get(url), HttpTimeout);
    REQUIRE_THROWS_CODE(connection->Post(url + "/names", "{}"), HttpTimeout);

    url = "192.0.2.0/files?action=GenerateDownloadInfo";

    REQUIRE_THROWS_CODE_MSG_MATCHES(connection->Get(url),
                                    HttpTimeout,
                                    Catch::Matchers::ContainsSubstring("timed out after"));
    REQUIRE_THROWS_CODE_MSG_MATCHES(connection->Post(url, {}),
                                    HttpTimeout,
                                    Catch::Matchers::ContainsSubstring("timed out after"));
}

TEST("Testing CurlConnection works from a second ConnectionManager")
{
    ReportingHandler handler;

    // Create a first connection manager and see it leave scope - curl initializes and uninitializes
    {
        CurlConnectionManager connectionManager(handler);
        REQUIRE(connectionManager.MakeConnection() != nullptr);
    }

    test::MockWebServer server;
    CurlConnectionManager connectionManager(handler);
    handler.SetLoggingCallback(LogCallbackToTest);
    auto connection = connectionManager.MakeConnection();

    const std::string url = SFSUrlComponents::GetSpecificVersionUrl(server.GetBaseUrl(),
                                                                    c_instanceId,
                                                                    c_namespace,
                                                                    c_productName,
                                                                    c_version);

    // Register the product
    server.RegisterProduct(c_productName, c_version);

    // After registering the product, the URL returns 200 OK
    REQUIRE_NOTHROW(connection->Get(url));

    SECTION("A second connection also works")
    {
        auto connection2 = connectionManager.MakeConnection();
        REQUIRE_NOTHROW(connection2->Get(url));
    }
}

TEST("Testing a url that's too big throws 414")
{
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);

    test::MockWebServer server;
    CurlConnectionManager connectionManager(handler);
    auto connection = connectionManager.MakeConnection();

    // Will use a fake large product name to produce a large url
    const std::string largeProductName(90000, 'a');

    // Register the products
    server.RegisterProduct(largeProductName, c_version);

    // Url produces: 414 URI Too Long
    std::string out;
    REQUIRE_THROWS_CODE_MSG(connection->Get(SFSUrlComponents::GetSpecificVersionUrl(server.GetBaseUrl(),
                                                                                    c_instanceId,
                                                                                    c_namespace,
                                                                                    largeProductName,
                                                                                    c_version)),
                            HttpUnexpected,
                            "Unexpected HTTP code 414");
}

TEST("Testing a response over the limit fails the operation")
{
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);

    test::MockWebServer server;
    CurlConnectionManager connectionManager(handler);
    auto connection = connectionManager.MakeConnection();

    // Will use a fake large product name to produce a response over the limit of 100k characters
    const std::string largeProductName(90000, 'a');
    const std::string overLimitProductName(1000000, 'a');

    // Register the products
    server.RegisterProduct(largeProductName, c_version);
    server.RegisterProduct(overLimitProductName, c_version);

    // Using GetLatestVersionBatch api since the product name is in the body and not in the url, to avoid a 414 error
    // like on the test above
    const std::string url = SFSUrlComponents::GetLatestVersionBatchUrl(server.GetBaseUrl(), c_instanceId, c_namespace);

    // Large one works
    json body = {{{"TargetingAttributes", {}}, {"Product", largeProductName}}};
    REQUIRE_NOTHROW(connection->Post(url, body.dump()));

    // Over limit fails
    body[0]["Product"] = overLimitProductName;
    REQUIRE_THROWS_CODE_MSG(connection->Post(url, body.dump()),
                            ConnectionUnexpectedError,
                            "Failure writing output to destination");
}
