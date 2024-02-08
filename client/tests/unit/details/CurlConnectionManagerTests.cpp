// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ReportingHandler.h"
#include "connection/Connection.h"
#include "connection/CurlConnection.h"
#include "connection/CurlConnectionManager.h"

#include <catch2/catch_test_macros.hpp>
#include <curl/curl.h>

using namespace SFS;
using namespace SFS::details;

#define TEST(...) TEST_CASE("[CurlConnectionManagerTests] " __VA_ARGS__)

TEST("Testing expected values in curl_version_info_data")
{
    curl_version_info_data* ver = curl_version_info(CURLVERSION_NOW);
    REQUIRE(ver != nullptr);
    CHECK(ver->features & CURL_VERSION_SSL);

    // Checking thread safety is on
    CHECK(ver->features & CURL_VERSION_THREADSAFE);

    // For thread safety we need the DNS resolutions to be asynchronous (which happens because of c-ares)
    CHECK(ver->features & CURL_VERSION_ASYNCHDNS);
}

TEST("Testing CurlConnectionManager()")
{
    ReportingHandler handler;
    std::unique_ptr<ConnectionManager> curlConnectionManager;
    REQUIRE(CurlConnectionManager::Make(handler, curlConnectionManager));

    // Check that the CurlConnectionManager generates a CurlConnection object
    std::unique_ptr<Connection> connection;
    REQUIRE(curlConnectionManager->MakeConnection(connection));
    REQUIRE(connection != nullptr);
    REQUIRE(dynamic_cast<CurlConnection*>(connection.get()) != nullptr);

    // Having many CurlConnectionManager objects should not cause any issues, curl is smart enough to
    // handle multiple initialization and cleanup calls
    std::unique_ptr<ConnectionManager> curlConnectionManager2;
    REQUIRE(CurlConnectionManager::Make(handler, curlConnectionManager2));
    std::unique_ptr<Connection> connection2;
    REQUIRE(curlConnectionManager2->MakeConnection(connection2));

    std::unique_ptr<ConnectionManager> curlConnectionManager3;
    REQUIRE(CurlConnectionManager::Make(handler, curlConnectionManager3));
    std::unique_ptr<Connection> connection3;
    REQUIRE(curlConnectionManager3->MakeConnection(connection3));
    std::unique_ptr<Connection> connection4;
    REQUIRE(curlConnectionManager3->MakeConnection(connection4));
}
