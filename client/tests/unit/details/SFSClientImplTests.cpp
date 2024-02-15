// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../util/SFSExceptionMatcher.h"
#include "../../util/TestHelper.h"
#include "SFSClientImpl.h"
#include "TestOverride.h"
#include "connection/Connection.h"
#include "connection/ConnectionManager.h"
#include "connection/CurlConnection.h"
#include "connection/CurlConnectionManager.h"
#include "connection/mock/MockConnectionManager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

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

    std::string Get(const std::string&) override
    {
        if (m_responseCode == Result::Success)
        {
            INFO("MockCurlConnection::Get() called, response: " << m_getResponse);
            return m_getResponse;
        }
        throw SFSException(m_responseCode);
    }

    std::string Post(const std::string&, const std::string& data) override
    {
        if (m_responseCode == Result::Success)
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
            return m_postResponse;
        }
        throw SFSException(m_responseCode);
    }

  private:
    Result::Code& m_responseCode;
    std::string& m_getResponse;
    std::string& m_postResponse;
    bool& m_expectEmptyPostBody;
};

void CheckProduct(const ContentId& contentId, std::string_view ns, std::string_view name, std::string_view version)
{
    REQUIRE(contentId.GetNameSpace() == ns);
    REQUIRE(contentId.GetName() == name);
    REQUIRE(contentId.GetVersion() == version);
}

void CheckDownloadInfo(const std::vector<File>& files, const std::string& name)
{
    REQUIRE(files.size() == 2);
    REQUIRE(files[0].GetFileId() == (name + ".json"));
    REQUIRE(files[0].GetUrl() == ("http://localhost/1.json"));
    REQUIRE(files[1].GetFileId() == (name + ".bin"));
    REQUIRE(files[1].GetUrl() == ("http://localhost/2.bin"));
}
} // namespace

TEST("Testing class SFSClientImpl()")
{
    const std::string ns = "testNameSpace";
    SFSClientImpl<CurlConnectionManager> sfsClient({"testAccountId", "testInstanceId", ns, LogCallbackToTest});

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
            {{"ContentId", {{"Namespace", ns}, {"Name", productName}, {"Version", expectedVersion}}}});
        postResponse = latestVersionResponse.dump();
        std::unique_ptr<ContentId> contentId;
        SECTION("No attributes")
        {
            REQUIRE(sfsClient.GetLatestVersion(productName, {}, *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, productName, expectedVersion);
        }

        SECTION("With attributes")
        {
            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE(sfsClient.GetLatestVersion(productName, attributes, *connection, contentId) == Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, productName, expectedVersion);
        }

        SECTION("Failing")
        {
            responseCode = Result::HttpNotFound;
            REQUIRE_THROWS_CODE(sfsClient.GetLatestVersion("badName", {}, *connection, contentId), HttpNotFound);

            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE_THROWS_CODE(sfsClient.GetLatestVersion("badName", attributes, *connection, contentId),
                                HttpNotFound);
        }
    }

    SECTION("Testing SFSClientImpl::GetSpecificVersion()")
    {
        json specificVersionResponse;
        specificVersionResponse["ContentId"] = {{"Namespace", ns}, {"Name", productName}, {"Version", expectedVersion}};
        specificVersionResponse["Files"] = json::array({productName + ".json", productName + ".bin"});
        getResponse = specificVersionResponse.dump();
        std::unique_ptr<ContentId> contentId;
        SECTION("Getting version")
        {
            REQUIRE(sfsClient.GetSpecificVersion(productName, expectedVersion, *connection, contentId) ==
                    Result::Success);
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, productName, expectedVersion);
        }

        SECTION("Failing")
        {
            responseCode = Result::HttpNotFound;
            REQUIRE_THROWS_CODE(sfsClient.GetSpecificVersion(productName, expectedVersion, *connection, contentId),
                                HttpNotFound);
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

        std::vector<File> files;
        SECTION("Getting version")
        {
            REQUIRE(sfsClient.GetDownloadInfo(productName, expectedVersion, *connection, files) == Result::Success);
            REQUIRE(!files.empty());
            CheckDownloadInfo(files, productName);
        }

        SECTION("Failing")
        {
            responseCode = Result::HttpNotFound;
            REQUIRE_THROWS_CODE(sfsClient.GetDownloadInfo(productName, expectedVersion, *connection, files),
                                HttpNotFound);
            REQUIRE(files.empty());
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

TEST("Testing test override SFS_TEST_OVERRIDE_BASE_URL")
{
    SFSClientImpl<MockConnectionManager> sfsClient({"testAccountId", "testInstanceId", "testNameSpace", std::nullopt});

    REQUIRE(sfsClient.GetBaseUrl() == "https://testAccountId.api.cdp.microsoft.com");

    {
        INFO("Can override the base url with the test key");
        ScopedTestOverride override(TestOverride::BaseUrl, "override");
        if (AreTestOverridesAllowed())
        {
            REQUIRE(sfsClient.GetBaseUrl() == "override");
        }
        else
        {
            REQUIRE(sfsClient.GetBaseUrl() == "https://testAccountId.api.cdp.microsoft.com");
        }
    }

    INFO("Override is unset after ScopedEnv goes out of scope");
    REQUIRE(sfsClient.GetBaseUrl() == "https://testAccountId.api.cdp.microsoft.com");

    sfsClient.SetCustomBaseUrl("customUrl");
    REQUIRE(sfsClient.GetBaseUrl() == "customUrl");

    {
        INFO("Can also override a custom base base url with the test key");
        ScopedTestOverride override(TestOverride::BaseUrl, "override");
        if (AreTestOverridesAllowed())
        {
            REQUIRE(sfsClient.GetBaseUrl() == "override");
        }
        else
        {
            REQUIRE(sfsClient.GetBaseUrl() == "customUrl");
        }
    }

    REQUIRE(sfsClient.GetBaseUrl() == "customUrl");
}

TEST("Testing passing a logging callback to constructor of SFSClientImpl")
{
    SFSClientImpl<MockConnectionManager> sfsClient(
        {"testAccountId", "testInstanceId", "testNameSpace", [](const LogData&) {}});
    SFSClientImpl<MockConnectionManager> sfsClient2({"testAccountId", "testInstanceId", "testNameSpace", nullptr});
}
