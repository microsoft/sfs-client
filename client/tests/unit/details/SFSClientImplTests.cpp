// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../util/TestHelper.h"
#include "Responses.h"
#include "SFSClientImpl.h"
#include "connection/CurlConnection.h"
#include "connection/CurlConnectionManager.h"
#include "connection/mock/MockConnectionManager.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[SFSClientImplTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;
using namespace SFS::test;
using json = nlohmann::json;

namespace
{
class MockCurlConnection : public CurlConnection
{
  public:
    MockCurlConnection(const ReportingHandler& handler,
                       Result::Code& responseCode,
                       std::string& getResponse,
                       std::string& postResponse,
                       bool& expectEmptyPostBody)
        : CurlConnection(handler)
        , m_responseCode(responseCode)
        , m_getResponse(getResponse)
        , m_postResponse(postResponse)
        , m_expectEmptyPostBody(expectEmptyPostBody)
    {
    }

    [[nodiscard]] Result Get(const std::string&, std::string& response) override
    {
        INFO("MockCurlConnection::Get() called, response: " << m_getResponse);
        response = m_getResponse;
        return m_responseCode;
    }

    [[nodiscard]] Result Post(const std::string&, const std::string& data, std::string& response) override
    {
        INFO("MockCurlConnection::Post() called, response: " << m_postResponse);
        if (m_expectEmptyPostBody)
        {
            REQUIRE(data.empty());
        }
        else
        {
            REQUIRE(!data.empty());
        }

        response = m_postResponse;
        return m_responseCode;
    }

  private:
    Result::Code& m_responseCode;
    std::string& m_getResponse;
    std::string& m_postResponse;
    bool& m_expectEmptyPostBody;
};

void CheckProduct(const json& product, std::string_view name, std::string_view version)
{
    INFO("Product: " << product.dump());
    REQUIRE(product.is_object());
    REQUIRE((product.contains("ContentId") && product.contains("Files")));
    REQUIRE((product["ContentId"].contains("Namespace") && product["ContentId"].contains("Name") &&
             product["ContentId"].contains("Version")));
    REQUIRE(product["Files"].is_array());
    REQUIRE(product["ContentId"]["Name"].get<std::string>() == name);
    REQUIRE(product["ContentId"]["Version"].get<std::string>() == version);
}

void CheckProductArray(const json& productArray, std::string_view name, std::string_view version)
{
    INFO("ProductArray: " << productArray.dump());
    REQUIRE(productArray.is_array());
    REQUIRE(productArray.size() == 1);

    const auto& product = productArray[0];
    REQUIRE((product.contains("ContentId")));
    REQUIRE((product["ContentId"].contains("Namespace") && product["ContentId"].contains("Name") &&
             product["ContentId"].contains("Version")));
    REQUIRE(product["ContentId"]["Name"].get<std::string>() == name);
    REQUIRE(product["ContentId"]["Version"].get<std::string>() == version);
}

void CheckDownloadInfo(const json& info, const std::string& name)
{
    INFO("DownloadInfo: " << info.dump());
    REQUIRE(info.is_array());
    REQUIRE(info.size() == 2);
    REQUIRE((info[0].contains("FileId") && info[1].contains("FileId")));
    REQUIRE(info[0]["FileId"].get<std::string>() == (name + ".json"));
    REQUIRE(info[1]["FileId"].get<std::string>() == (name + ".bin"));
}
} // namespace

TEST("Testing class SFSClientImpl()")
{
    SFSClientImpl<CurlConnectionManager> sfsClient(
        {"testAccountId", "testInstanceId", "testNameSpace", LogCallbackToTest});

    Result::Code responseCode = Result::Success;
    std::string getResponse;
    std::string postResponse;
    bool expectEmptyPostBody = true;
    std::unique_ptr<Connection> connection = std::make_unique<MockCurlConnection>(sfsClient.GetReportingHandler(),
                                                                                  responseCode,
                                                                                  getResponse,
                                                                                  postResponse,
                                                                                  expectEmptyPostBody);

    const std::string productName = "productName";
    const std::string expectedVersion = "0.0.0.2";
    SECTION("Testing SFSClientImpl::GetLatestVersion()")
    {
        expectEmptyPostBody = false;
        json latestVersionResponse = json::array();
        latestVersionResponse.push_back(
            {{"ContentId", {{"Namespace", "testNameSpace"}, {"Name", productName}, {"Version", expectedVersion}}}});
        postResponse = latestVersionResponse.dump();
        std::unique_ptr<VersionResponse> response;
        SECTION("No attributes")
        {
            REQUIRE(sfsClient.GetLatestVersion(productName, std::nullopt, *connection, response) == Result::Success);
            CheckProductArray(response->GetResponseData(), productName, expectedVersion);
        }

        SECTION("With attributes")
        {
            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE(sfsClient.GetLatestVersion(productName, attributes, *connection, response) == Result::Success);
            CheckProductArray(response->GetResponseData(), productName, expectedVersion);
        }

        SECTION("Failing")
        {
            responseCode = Result::HttpNotFound;
            REQUIRE(sfsClient.GetLatestVersion("badName", std::nullopt, *connection, response) == responseCode);

            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE(sfsClient.GetLatestVersion("badName", attributes, *connection, response) == responseCode);
        }
    }

    SECTION("Testing SFSClientImpl::GetSpecificVersion()")
    {
        json specificVersionResponse;
        specificVersionResponse["ContentId"] = {{"Namespace", "testNameSpace"},
                                                {"Name", productName},
                                                {"Version", expectedVersion}};
        specificVersionResponse["Files"] = json::array({productName + ".json", productName + ".bin"});
        getResponse = specificVersionResponse.dump();
        std::unique_ptr<VersionResponse> response;
        SECTION("Getting version")
        {
            REQUIRE(sfsClient.GetSpecificVersion(productName, expectedVersion, *connection, response) ==
                    Result::Success);
            CheckProduct(response->GetResponseData(), productName, expectedVersion);
        }

        SECTION("Failing")
        {
            responseCode = Result::HttpNotFound;
            REQUIRE(sfsClient.GetSpecificVersion(productName, expectedVersion, *connection, response) == responseCode);
        }
    }

    SECTION("Testing SFSClientImpl::GetDownloadInfo()")
    {
        expectEmptyPostBody = true;
        json downloadInfoResponse;
        downloadInfoResponse = json::array();
        downloadInfoResponse.push_back({{"Url", "http://localhost/1.json"},
                                        {"FileId", productName + ".json"},
                                        {"SizeInBytes", 100},
                                        {"Hashes", {{"Sha1", "123"}, {"Sha256", "456"}}}});
        downloadInfoResponse[0]["DeliveryOptimization"] = {{"CatalogId", "789"}};
        downloadInfoResponse[0]["DeliveryOptimization"]["Properties"] = {
            {"IntegrityCheckInfo", {{"PiecesHashFileUrl", "http://localhost/1.json"}, {"HashOfHashes", "abc"}}}};

        downloadInfoResponse.push_back({{"Url", "http://localhost/2.bin"},
                                        {"FileId", productName + ".bin"},
                                        {"SizeInBytes", 200},
                                        {"Hashes", {{"Sha1", "421"}, {"Sha256", "132"}}}});
        downloadInfoResponse[1]["DeliveryOptimization"] = downloadInfoResponse[0]["DeliveryOptimization"];
        postResponse = downloadInfoResponse.dump();

        std::unique_ptr<DownloadInfoResponse> response;
        SECTION("Getting version")
        {
            REQUIRE(sfsClient.GetDownloadInfo(productName, expectedVersion, *connection, response) == Result::Success);
            CheckDownloadInfo(response->GetResponseData(), productName);
        }

        SECTION("Failing")
        {
            responseCode = Result::HttpNotFound;
            REQUIRE(sfsClient.GetDownloadInfo(productName, expectedVersion, *connection, response) == responseCode);
        }
    }
}

TEST("Testing SFSClientImpl::SetCustomBaseUrl()")
{
    SFSClientImpl<MockConnectionManager> sfsClient({"testAccountId", "testInstanceId", "testNameSpace", std::nullopt});

    REQUIRE(sfsClient.GetBaseUrl() == "https://testAccountId.api.cdp.microsoft.com");

    sfsClient.SetCustomBaseUrl("customUrl");
    REQUIRE(sfsClient.GetBaseUrl() == "customUrl");
}

TEST("Testing passing a logging callback to constructor of SFSClientImpl")
{
    SFSClientImpl<MockConnectionManager> sfsClient(
        {"testAccountId", "testInstanceId", "testNameSpace", [](const LogData&) {}});
    SFSClientImpl<MockConnectionManager> sfsClient2({"testAccountId", "testInstanceId", "testNameSpace", nullptr});
}
