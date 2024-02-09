// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../mock/MockWebServer.h"
#include "../../util/TestHelper.h"
#include "ReportingHandler.h"
#include "SFSUrlComponents.h"
#include "connection/CurlConnection.h"
#include "connection/CurlConnectionManager.h"

#include <catch2/catch_test_macros.hpp>
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
    [[nodiscard]] static Result Make(const ReportingHandler& handler, std::unique_ptr<Connection>& out)
    {
        auto tmp = std::unique_ptr<CurlConnectionTimeout>(new CurlConnectionTimeout(handler));
        REQUIRE(tmp->SetupCurl());
        out = std::move(tmp);
        return Result::Success;
    }

    [[nodiscard]] Result Get(const std::string& url, std::string& response) override
    {
        // Timeout within 100ms
        curl_easy_setopt(m_handle, CURLOPT_TIMEOUT_MS, 100L);
        return CurlConnection::Get(url, response);
    }

    [[nodiscard]] Result Post(const std::string& url, const std::string& data, std::string& response) override
    {
        // Timeout within 100ms
        curl_easy_setopt(m_handle, CURLOPT_TIMEOUT_MS, 100L);
        return CurlConnection::Post(url, data, response);
    }

  private:
    CurlConnectionTimeout(const ReportingHandler& handler) : CurlConnection(handler)
    {
    }
};

class CurlConnectionTimeoutManager : public CurlConnectionManager
{
  public:
    [[nodiscard]] static Result Make(const ReportingHandler& handler, std::unique_ptr<ConnectionManager>& out)
    {
        auto tmp = std::unique_ptr<CurlConnectionTimeoutManager>(new CurlConnectionTimeoutManager(handler));
        REQUIRE(tmp->SetupCurl());
        out = std::move(tmp);

        return Result::Success;
    }

    [[nodiscard]] Result MakeConnection(std::unique_ptr<Connection>& out) override
    {
        REQUIRE(CurlConnectionTimeout::Make(m_handler, out));
        return Result::Success;
    }

  private:
    CurlConnectionTimeoutManager(const ReportingHandler& handler) : CurlConnectionManager(handler)
    {
    }
};
} // namespace

TEST("Testing CurlConnection()")
{
    test::MockWebServer server;
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);
    std::unique_ptr<ConnectionManager> connectionManager;
    REQUIRE(CurlConnectionManager::Make(handler, connectionManager));
    std::unique_ptr<Connection> connection;
    REQUIRE(connectionManager->MakeConnection(connection));

    SECTION("Testing CurlConnection::Get()")
    {
        const std::string url = SFSUrlComponents::GetSpecificVersionUrl(server.GetBaseUrl(),
                                                                        c_instanceId,
                                                                        c_namespace,
                                                                        c_productName,
                                                                        c_version);

        // Before registering the product, the URL returns 404 Not Found
        std::string out;
        REQUIRE(connection->Get(url, out) == Result::HttpNotFound);

        // Register the product
        server.RegisterProduct(c_productName, c_version);

        // After registering the product, the URL returns 200 OK
        REQUIRE(connection->Get(url, out) == Result::Success);

        json expectedResponse;
        expectedResponse["ContentId"] = {{"Namespace", "default"}, {"Name", c_productName}, {"Version", c_version}};
        expectedResponse["Files"] = json::array({c_productName + ".json", c_productName + ".bin"});

        REQUIRE(json::parse(out) == expectedResponse);
    }

    SECTION("Testing CurlConnection::Post()")
    {
        SECTION("With GetLatestVersion mock")
        {
            const std::string url =
                SFSUrlComponents::GetLatestVersionUrl(server.GetBaseUrl(), c_instanceId, c_namespace);

            std::string out;

            // Missing proper body returns HttpBadRequest
            REQUIRE(connection->Post(url, out) == Result::HttpBadRequest);

            // Before registering the product, the URL returns 404 Not Found
            const json body = {{{"TargetingAttributes", {}}, {"Product", c_productName}}};
            REQUIRE(connection->Post(url, body.dump(), out) == Result::HttpNotFound);

            // Register the product
            server.RegisterProduct(c_productName, c_version);

            // After registering the product, the URL returns 200 OK
            REQUIRE(connection->Post(url, body.dump(), out) == Result::Success);

            json expectedResponse = json::array();
            expectedResponse.push_back(
                {{"ContentId", {{"Namespace", "default"}, {"Name", c_productName}, {"Version", c_version}}}});
            REQUIRE(json::parse(out) == expectedResponse);

            // Returns the next version now
            server.RegisterProduct(c_productName, c_nextVersion);
            REQUIRE(connection->Post(url, body.dump(), out) == Result::Success);

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
            std::string out;
            REQUIRE(connection->Post(url, out) == Result::HttpNotFound);

            // Register the product
            server.RegisterProduct(c_productName, c_version);

            // After registering the product, the URL returns 200 OK
            REQUIRE(connection->Post(url, out) == Result::Success);

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
        std::string out;
        REQUIRE(connection->Get(url, out) == Result::HttpNotFound);
        CHECK(out.empty());
        REQUIRE(connection->Get(url + "/names", out) == Result::HttpNotFound);
        REQUIRE(connection->Post(url + "/names", "{}", out) == Result::HttpNotFound);

        url = server.GetBaseUrl() + "/api/v1/contents/" + c_instanceId + "/namespaces/" + c_namespace + "/names/" +
              c_productName + "/versYions/" + c_version + "/files?action=GenerateDownloadInfo";

        REQUIRE(connection->Get(url, out) == Result::HttpNotFound);
        REQUIRE(connection->Post(url, std::string(), out) == Result::HttpNotFound);
        REQUIRE(connection->Post(url, {}, out) == Result::HttpNotFound);
        CHECK(out.empty());
    }

    REQUIRE(server.Stop() == Result::Success);
}

TEST("Testing CurlConnection when the server is not reachable")
{
    // Using a custom override class just to time out faster on an invalid URL
    ReportingHandler handler;
    std::unique_ptr<ConnectionManager> connectionManager;
    REQUIRE(CurlConnectionTimeoutManager::Make(handler, connectionManager));
    handler.SetLoggingCallback(LogCallbackToTest);
    std::unique_ptr<Connection> connection;
    REQUIRE(connectionManager->MakeConnection(connection));

    // Using a non-routable IP address to ensure the server is not reachable
    // https://www.rfc-editor.org/rfc/rfc5737#section-3: The blocks 192.0.2.0/24 (...) are provided for use in
    // documentation.
    std::string url = "192.0.2.0";
    std::string out;
    REQUIRE(connection->Get(url, out) == Result::HttpTimeout);
    REQUIRE(connection->Post(url + "/names", "{}", out) == Result::HttpTimeout);

    url = "192.0.2.0/files?action=GenerateDownloadInfo";

    auto ret = connection->Get(url, out);
    REQUIRE(ret.GetCode() == Result::HttpTimeout);
    REQUIRE(ret.GetMessage().find("timed out after") != std::string::npos);

    ret = connection->Post(url, {}, out);
    REQUIRE(ret.GetCode() == Result::HttpTimeout);
    REQUIRE(ret.GetMessage().find("timed out after") != std::string::npos);
}

TEST("Testing CurlConnection works from a second ConnectionManager")
{
    ReportingHandler handler;

    // Create a first connection manager and see it leave scope - curl initializes and uninitializes
    {
        std::unique_ptr<ConnectionManager> connectionManager;
        REQUIRE(CurlConnectionManager::Make(handler, connectionManager));
        std::unique_ptr<Connection> connection;
        REQUIRE(connectionManager->MakeConnection(connection));
    }

    test::MockWebServer server;
    std::unique_ptr<ConnectionManager> connectionManager;
    REQUIRE(CurlConnectionManager::Make(handler, connectionManager));
    handler.SetLoggingCallback(LogCallbackToTest);
    std::unique_ptr<Connection> connection;
    REQUIRE(connectionManager->MakeConnection(connection));

    const std::string url = SFSUrlComponents::GetSpecificVersionUrl(server.GetBaseUrl(),
                                                                    c_instanceId,
                                                                    c_namespace,
                                                                    c_productName,
                                                                    c_version);

    // Register the product
    server.RegisterProduct(c_productName, c_version);

    // After registering the product, the URL returns 200 OK
    std::string out;
    REQUIRE(connection->Get(url, out) == Result::Success);

    SECTION("A second connection also works")
    {
        std::unique_ptr<Connection> connection2;
        REQUIRE(connectionManager->MakeConnection(connection2));
        REQUIRE(connection2->Get(url, out) == Result::Success);
    }
}

TEST("Testing a url that's too big throws 414")
{
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);

    test::MockWebServer server;
    std::unique_ptr<ConnectionManager> connectionManager;
    REQUIRE(CurlConnectionManager::Make(handler, connectionManager));
    std::unique_ptr<Connection> connection;
    REQUIRE(connectionManager->MakeConnection(connection));

    // Will use a fake large product name to produce a large url
    const std::string largeProductName(90000, 'a');

    // Register the products
    server.RegisterProduct(largeProductName, c_version);

    // Url produces: 414 URI Too Long
    std::string out;
    auto ret = connection->Get(SFSUrlComponents::GetSpecificVersionUrl(server.GetBaseUrl(),
                                                                       c_instanceId,
                                                                       c_namespace,
                                                                       largeProductName,
                                                                       c_version),
                               out);
    REQUIRE(ret.GetCode() == Result::HttpUnexpected);
    REQUIRE(ret.GetMessage() == "Unexpected HTTP code 414");
}

TEST("Testing a response over the limit fails the operation")
{
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);

    test::MockWebServer server;
    std::unique_ptr<ConnectionManager> connectionManager;
    REQUIRE(CurlConnectionManager::Make(handler, connectionManager));
    std::unique_ptr<Connection> connection;
    REQUIRE(connectionManager->MakeConnection(connection));

    // Will use a fake large product name to produce a response over the limit of 100k characters
    const std::string largeProductName(90000, 'a');
    const std::string overLimitProductName(1000000, 'a');

    // Register the products
    server.RegisterProduct(largeProductName, c_version);
    server.RegisterProduct(overLimitProductName, c_version);

    // Using GetLatestVersion api since the product name is in the body and not in the url, to avoid a 414 error like on
    // the test above
    const std::string url = SFSUrlComponents::GetLatestVersionUrl(server.GetBaseUrl(), c_instanceId, c_namespace);

    // Large one works
    json body = {{{"TargetingAttributes", {}}, {"Product", largeProductName}}};
    std::string out;
    REQUIRE(connection->Post(url, body.dump(), out) == Result::Success);

    // Over limit fails
    body[0]["Product"] = overLimitProductName;
    auto ret = connection->Post(url, body.dump(), out);
    REQUIRE(ret.GetCode() == Result::ConnectionUnexpectedError);
    REQUIRE(ret.GetMessage() == "Failure writing output to destination");
}
