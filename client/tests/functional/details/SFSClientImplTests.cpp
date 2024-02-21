// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../mock/MockWebServer.h"
#include "../../util/SFSExceptionMatcher.h"
#include "../../util/TestHelper.h"
#include "SFSClientImpl.h"
#include "connection/Connection.h"
#include "connection/CurlConnectionManager.h"

#include <catch2/catch_test_macros.hpp>

#include <set>

#define TEST(...) TEST_CASE("[Functional][SFSClientImplTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;
using namespace SFS::test;

namespace
{
void CheckProduct(const ContentId& contentId, std::string_view ns, std::string_view name, std::string_view version)
{
    REQUIRE(contentId.GetNameSpace() == ns);
    REQUIRE(contentId.GetName() == name);
    REQUIRE(contentId.GetVersion() == version);
}

void CheckProducts(const std::vector<ContentId>& contentIds,
                   std::string_view ns,
                   const std::set<std::pair<std::string, std::string>>& nameVersionPairs)
{
    // json arrays don't guarantee order, and they are the underlying structure, so we need to check using sets
    std::set<std::pair<std::string, std::string>> uniqueNameVersionPairs;
    for (const auto& contentId : contentIds)
    {
        REQUIRE(contentId.GetNameSpace() == ns);
        uniqueNameVersionPairs.emplace(contentId.GetName(), contentId.GetVersion());
    }

    for (const auto& nameVersionPair : nameVersionPairs)
    {
        REQUIRE(uniqueNameVersionPairs.count(nameVersionPair));
    }
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
    test::MockWebServer server;
    const std::string ns = "testNameSpace";
    SFSClientImpl<CurlConnectionManager> sfsClient({"testAccountId", "testInstanceId", ns, LogCallbackToTest});
    sfsClient.SetCustomBaseUrl(server.GetBaseUrl());

    server.RegisterProduct("productName", "0.0.0.2");
    server.RegisterProduct("productName", "0.0.0.1");

    auto connection = sfsClient.GetConnectionManager().MakeConnection();

    SECTION("Testing SFSClientImpl::GetLatestVersion()")
    {
        std::unique_ptr<ContentId> contentId;

        SECTION("No attributes")
        {
            REQUIRE_NOTHROW(contentId = sfsClient.GetLatestVersion({"productName", {}}, *connection));
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.2");
        }

        SECTION("With attributes")
        {
            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE_NOTHROW(contentId = sfsClient.GetLatestVersion({"productName", attributes}, *connection));
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.2");
        }

        SECTION("Wrong product name")
        {
            REQUIRE_THROWS_CODE(contentId = sfsClient.GetLatestVersion({"badName", {}}, *connection), HttpNotFound);
            REQUIRE(!contentId);

            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE_THROWS_CODE(contentId = sfsClient.GetLatestVersion({"badName", attributes}, *connection),
                                HttpNotFound);
            REQUIRE(!contentId);
        }
    }

    SECTION("Testing SFSClientImpl::GetLatestVersionBatch()")
    {
        std::vector<ContentId> contentIds;

        SECTION("No attributes")
        {
            REQUIRE_NOTHROW(contentIds = sfsClient.GetLatestVersionBatch({{"productName", {}}}, *connection));
            REQUIRE(!contentIds.empty());
            CheckProduct(contentIds[0], ns, "productName", "0.0.0.2");
        }

        SECTION("With attributes")
        {
            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE_NOTHROW(contentIds = sfsClient.GetLatestVersionBatch({{"productName", attributes}}, *connection));
            REQUIRE(!contentIds.empty());
            CheckProduct(contentIds[0], ns, "productName", "0.0.0.2");
        }

        SECTION("Wrong product name")
        {
            REQUIRE_THROWS_CODE(contentIds = sfsClient.GetLatestVersionBatch({{"badName", {}}}, *connection),
                                HttpNotFound);
            REQUIRE(contentIds.empty());

            const SearchAttributes attributes{{"attr1", "value"}};
            REQUIRE_THROWS_CODE(contentIds = sfsClient.GetLatestVersionBatch({{"badName", attributes}}, *connection),
                                HttpNotFound);
            REQUIRE(contentIds.empty());
        }

        SECTION("Multiple unique products")
        {
            server.RegisterProduct("productName2", "0.0.0.3");

            REQUIRE_NOTHROW(
                contentIds = sfsClient.GetLatestVersionBatch({{"productName", {}}, {"productName2", {}}}, *connection));
            REQUIRE(contentIds.size() == 2);
            CheckProducts(contentIds, ns, {{"productName", "0.0.0.2"}, {"productName2", "0.0.0.3"}});

            server.RegisterProduct("productName3", "0.0.0.4");

            REQUIRE_NOTHROW(contentIds = sfsClient.GetLatestVersionBatch(
                                {{"productName", {}}, {"productName2", {}}, {"productName3", {}}},
                                *connection));
            REQUIRE(contentIds.size() == 3);
            CheckProducts(contentIds,
                          ns,
                          {{"productName", "0.0.0.2"}, {"productName2", "0.0.0.3"}, {"productName3", "0.0.0.4"}});
        }

        SECTION("Multiple repeated products")
        {
            REQUIRE_NOTHROW(
                contentIds = sfsClient.GetLatestVersionBatch({{"productName", {}}, {"productName", {}}}, *connection));
            REQUIRE(contentIds.size() == 1);
            CheckProduct(contentIds[0], ns, "productName", "0.0.0.2");

            server.RegisterProduct("productName2", "0.0.0.3");

            REQUIRE_NOTHROW(contentIds = sfsClient.GetLatestVersionBatch(
                                {{"productName", {}}, {"productName", {}}, {"productName2", {}}, {"productName2", {}}},
                                *connection));
            REQUIRE(contentIds.size() == 2);
            CheckProducts(contentIds, ns, {{"productName", "0.0.0.2"}, {"productName2", "0.0.0.3"}});
        }

        SECTION("Multiple wrong products returns 404")
        {
            REQUIRE_THROWS_CODE(contentIds =
                                    sfsClient.GetLatestVersionBatch({{"badName", {}}, {"badName2", {}}}, *connection),
                                HttpNotFound);
            REQUIRE(contentIds.empty());
        }

        SECTION("Multiple products, one wrong returns 200")
        {
            REQUIRE_NOTHROW(contentIds =
                                sfsClient.GetLatestVersionBatch({{"productName", {}}, {"badName", {}}}, *connection));
            REQUIRE(contentIds.size() == 1);
            CheckProduct(contentIds[0], ns, "productName", "0.0.0.2");
        }
    }

    SECTION("Testing SFSClientImpl::GetSpecificVersion()")
    {
        std::unique_ptr<ContentId> contentId;

        SECTION("Getting 0.0.0.1")
        {
            REQUIRE_NOTHROW(contentId = sfsClient.GetSpecificVersion("productName", "0.0.0.1", *connection));
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.1");
        }

        SECTION("Getting 0.0.0.2")
        {
            REQUIRE_NOTHROW(contentId = sfsClient.GetSpecificVersion("productName", "0.0.0.2", *connection));
            REQUIRE(contentId);
            CheckProduct(*contentId, ns, "productName", "0.0.0.2");
        }

        SECTION("Wrong product name")
        {
            REQUIRE_THROWS_CODE(contentId = sfsClient.GetSpecificVersion("badName", "0.0.0.2", *connection),
                                HttpNotFound);
            REQUIRE(!contentId);
        }

        SECTION("Wrong version")
        {
            REQUIRE_THROWS_CODE(contentId = sfsClient.GetSpecificVersion("productName", "0.0.0.3", *connection),
                                HttpNotFound);
            REQUIRE(!contentId);
        }
    }

    SECTION("Testing SFSClientImpl::GetDownloadInfo()")
    {
        std::vector<File> files;

        SECTION("Succesful operations")
        {
            std::string version;
            SECTION("Getting 0.0.0.1")
            {
                version = "0.0.0.1";
            }

            SECTION("Getting 0.0.0.2")
            {
                version = "0.0.0.2";
            }

            REQUIRE_NOTHROW(files = sfsClient.GetDownloadInfo("productName", version, *connection));
            REQUIRE(!files.empty());
            CheckDownloadInfo(files, "productName");

            INFO("Checking DOData is present");
            std::unique_ptr<ContentId> contentId;
            REQUIRE(ContentId::Make(ns, "productName", version, contentId));
            for (const auto& file : files)
            {
                std::unique_ptr<DeliveryOptimizationData> data;
                REQUIRE_NOTHROW(data = sfsClient.GetDeliveryOptimizationData(*contentId, file));
                REQUIRE(data);

                INFO("We only know the server doesn't send the catalogId and properties empty");
                REQUIRE(!data->GetCatalogId().empty());
                REQUIRE(!data->GetProperties().empty());
            }
        }

        SECTION("Wrong product name")
        {
            REQUIRE_THROWS_CODE(files = sfsClient.GetDownloadInfo("badName", "0.0.0.2", *connection), HttpNotFound);
            REQUIRE(files.empty());
        }

        SECTION("Wrong version")
        {
            REQUIRE_THROWS_CODE(files = sfsClient.GetDownloadInfo("productName", "0.0.0.3", *connection), HttpNotFound);
            REQUIRE(files.empty());
        }
    }

    REQUIRE(server.Stop() == Result::Success);
}
