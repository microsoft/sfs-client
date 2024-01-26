// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../mock/MockWebServer.h"
#include "../../util/TestHelper.h"
#include "ReportingHandler.h"
#include "Util.h"
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
    CurlConnectionTimeout(const ReportingHandler& handler) : CurlConnection(handler)
    {
    }

    [[nodiscard]] Result Get(std::string_view url, std::string& response) override
    {
        // Timeout within 100ms
        curl_easy_setopt(m_handle, CURLOPT_TIMEOUT_MS, 100L);
        return CurlConnection::Get(url, response);
    }

    [[nodiscard]] Result Post(std::string_view url, std::string_view data, std::string& response) override
    {
        // Timeout within 100ms
        curl_easy_setopt(m_handle, CURLOPT_TIMEOUT_MS, 100L);
        return CurlConnection::Post(url, data, response);
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
        const std::string url =
            util::GetSpecificVersionUrl(server.GetBaseUrl(), c_instanceId, c_namespace, c_productName, c_version);

        // Before registering the product, the URL returns 404 Not Found
        std::string out;
        REQUIRE(connection->Get(url, out) == Result::E_HttpNotFound);

        // Register the product
        server.RegisterProduct(c_productName, c_version);

        // After registering the product, the URL returns 200 OK
        REQUIRE(connection->Get(url, out) == Result::S_Ok);

        json expectedResponse;
        expectedResponse["ContentId"] = {{"Namespace", "default"}, {"Name", c_productName}, {"Version", c_version}};
        expectedResponse["Files"] = json::array({c_productName + ".json", c_productName + ".bin"});

        REQUIRE(json::parse(out) == expectedResponse);
    }

    SECTION("Testing CurlConnection::Post()")
    {
        SECTION("With GetLatestVersion mock")
        {
            const std::string url = util::GetLatestVersionUrl(server.GetBaseUrl(), c_instanceId, c_namespace);

            std::string out;

            // Missing proper body returns E_HttpBadRequest
            REQUIRE(connection->Post(url, out) == Result::E_HttpBadRequest);

            // Before registering the product, the URL returns 404 Not Found
            const json body = {{{"TargetingAttributes", {}}, {"Product", c_productName}}};
            REQUIRE(connection->Post(url, body.dump(), out) == Result::E_HttpNotFound);

            // Register the product
            server.RegisterProduct(c_productName, c_version);

            // After registering the product, the URL returns 200 OK
            REQUIRE(connection->Post(url, body.dump(), out) == Result::S_Ok);

            json expectedResponse = json::array();
            expectedResponse.push_back(
                {{"ContentId", {{"Namespace", "default"}, {"Name", c_productName}, {"Version", c_version}}}});
            REQUIRE(json::parse(out) == expectedResponse);

            // Returns the next version now
            server.RegisterProduct(c_productName, c_nextVersion);
            REQUIRE(connection->Post(url, body.dump(), out) == Result::S_Ok);

            expectedResponse = json::array();
            expectedResponse.push_back(
                {{"ContentId", {{"Namespace", "default"}, {"Name", c_productName}, {"Version", c_nextVersion}}}});
            REQUIRE(json::parse(out) == expectedResponse);
        }

        SECTION("With GetDownloadInfo mock")
        {
            const std::string url =
                util::GetDownloadInfoUrl(server.GetBaseUrl(), c_instanceId, c_namespace, c_productName, c_version);

            // Before registering the product, the URL returns 404 Not Found
            std::string out;
            REQUIRE(connection->Post(url, out) == Result::E_HttpNotFound);

            // Register the product
            server.RegisterProduct(c_productName, c_version);

            // After registering the product, the URL returns 200 OK
            REQUIRE(connection->Post(url, out) == Result::S_Ok);

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
        REQUIRE(connection->Get(url, out) == Result::E_HttpNotFound);
        CHECK(out.empty());
        REQUIRE(connection->Get(url + "/names", out) == Result::E_HttpNotFound);
        REQUIRE(connection->Post(url + "/names", "{}", out) == Result::E_HttpNotFound);

        url = server.GetBaseUrl() + "/api/v1/contents/" + c_instanceId + "/namespaces/" + c_namespace + "/names/" +
              c_productName + "/versYions/" + c_version + "/files?action=GenerateDownloadInfo";

        REQUIRE(connection->Get(url, out) == Result::E_HttpNotFound);
        REQUIRE(connection->Post(url, std::string(), out) == Result::E_HttpNotFound);
        REQUIRE(connection->Post(url, {}, out) == Result::E_HttpNotFound);
        CHECK(out.empty());
    }

    REQUIRE(server.Stop() == Result::S_Ok);
}

TEST("Testing CurlConnection when the server is not reachable")
{
    // Using a custom override class just to time out faster on an invalid URL
    ReportingHandler handler;
    CurlConnectionTimeoutManager connectionManager(handler);
    handler.SetLoggingCallback(LogCallbackToTest);
    auto connection = connectionManager.MakeConnection();

    std::string url = "dummy";
    std::string out;
    REQUIRE(connection->Get(url, out) == Result::E_HttpTimeout);
    REQUIRE(connection->Post(url + "/names", "{}", out) == Result::E_HttpTimeout);

    url = "dummy/files?action=GenerateDownloadInfo";

    auto ret = connection->Get(url, out);
    REQUIRE(ret.GetCode() == Result::E_HttpTimeout);
    REQUIRE(ret.GetMessage().find("timed out after") != std::string::npos);

    ret = connection->Post(url, {}, out);
    REQUIRE(ret.GetCode() == Result::E_HttpTimeout);
    REQUIRE(ret.GetMessage().find("timed out after") != std::string::npos);
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

    const std::string url =
        util::GetSpecificVersionUrl(server.GetBaseUrl(), c_instanceId, c_namespace, c_productName, c_version);

    // Register the product
    server.RegisterProduct(c_productName, c_version);

    // After registering the product, the URL returns 200 OK
    std::string out;
    REQUIRE(connection->Get(url, out) == Result::S_Ok);

    SECTION("A second connection also works")
    {
        auto connection2 = connectionManager.MakeConnection();
        REQUIRE(connection2->Get(url, out) == Result::S_Ok);
    }
}
