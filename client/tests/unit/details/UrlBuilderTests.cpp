// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../util/SFSExceptionMatcher.h"
#include "../../util/TestHelper.h"
#include "ReportingHandler.h"
#include "UrlBuilder.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[UrlBuilderTests] " __VA_ARGS__)

using namespace SFS::details;
using namespace SFS::test;

TEST("UrlBuilder")
{
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);

    UrlBuilder builder(handler);
    REQUIRE_THROWS_CODE_MSG(builder.GetUrl(), ConnectionUrlSetupFailed, "Curl URL error: No host part in the URL");

    builder.SetHost("www.example.com");
    REQUIRE_THROWS_CODE_MSG(builder.GetUrl(), ConnectionUrlSetupFailed, "Curl URL error: No scheme part in the URL");

    builder.SetScheme(Scheme::Https);
    REQUIRE(builder.GetUrl() == "https://www.example.com/");

    SECTION("SetHost")
    {
        builder.SetHost("www.example2.com");
        REQUIRE(builder.GetUrl() == "https://www.example2.com/");

        REQUIRE_THROWS_CODE_MSG(builder.SetHost("+"), ConnectionUrlSetupFailed, "Curl URL error: Bad hostname");
        REQUIRE_THROWS_CODE_MSG(builder.SetHost("a&b"), ConnectionUrlSetupFailed, "Curl URL error: Bad hostname");
        REQUIRE_THROWS_CODE_MSG(builder.SetHost("a\nb"), ConnectionUrlSetupFailed, "Curl URL error: Bad hostname");
        REQUIRE_THROWS_CODE_MSG(builder.SetHost("a\tb"), ConnectionUrlSetupFailed, "Curl URL error: Bad hostname");
    }

    SECTION("SetPath")
    {
        builder.SetPath("index.html");
        REQUIRE(builder.GetUrl() == "https://www.example.com/index.html");

        builder.SetPath("index.html", true);
        REQUIRE(builder.GetUrl() == "https://www.example.com/index.html");

        builder.SetPath("index>@", true);
        REQUIRE(builder.GetUrl() == "https://www.example.com/index%3e%40");

        INFO("Encoding skips the / character");
        builder.SetPath("index>@/index", true);
        REQUIRE(builder.GetUrl() == "https://www.example.com/index%3e%40/index");
    }

    SECTION("AppendPath")
    {
        builder.SetPath("index.html");
        REQUIRE(builder.GetUrl() == "https://www.example.com/index.html");

        builder.AppendPath("index.html", true);
        REQUIRE(builder.GetUrl() == "https://www.example.com/index.html/index.html");

        builder.AppendPath("a/", false);
        REQUIRE(builder.GetUrl() == "https://www.example.com/index.html/index.html/a/");

        builder.AppendPath("b/", false);
        REQUIRE(builder.GetUrl() == "https://www.example.com/index.html/index.html/a/b/");

        INFO("Encoding for append includes the / character");
        builder.AppendPath("c/", true);
        REQUIRE(builder.GetUrl() == "https://www.example.com/index.html/index.html/a/b/c%2f");

        INFO("Calling SetPath() resets the path");
        builder.SetPath("index>@", true);
        REQUIRE(builder.GetUrl() == "https://www.example.com/index%3e%40");
    }

    SECTION("SetQuery, AppendQuery")
    {
        builder.SetQuery("key=value");
        REQUIRE(builder.GetUrl() == "https://www.example.com/?key=value");

        builder.AppendQuery("key2=value2");
        REQUIRE(builder.GetUrl() == "https://www.example.com/?key=value&key2=value2");

        builder.SetQuery("key2=value2");
        REQUIRE(builder.GetUrl() == "https://www.example.com/?key2=value2");
    }

    SECTION("SetUrl")
    {
        builder.SetUrl("https://www.example.com/index.html?key=value");
        REQUIRE(builder.GetUrl() == "https://www.example.com/index.html?key=value");
        REQUIRE_THROWS_CODE_MSG(builder.SetUrl("https://www.+.com"),
                                ConnectionUrlSetupFailed,
                                "Curl URL error: Bad hostname");
    }

    SECTION("SetScheme, SetHost, SetPath, SetQuery")
    {
        builder.SetScheme(Scheme::Https);
        builder.SetHost("www.example.com");
        builder.SetPath("index.html");
        builder.SetQuery("key=value");

        REQUIRE(builder.GetUrl() == "https://www.example.com/index.html?key=value");
    }

    SECTION("SetScheme, SetHost, SetPath, SetQuery with encoding")
    {
        builder.SetScheme(Scheme::Https);
        builder.SetHost("www.example.com");
        builder.SetPath("index@.html", true);
        builder.SetQuery("key=value");

        REQUIRE(builder.GetUrl() == "https://www.example.com/index%40.html?key=value");
    }

    SECTION("Constructor with URL")
    {
        UrlBuilder builder2("https://www.example.com/index.html?key=value", handler);
        REQUIRE(builder2.GetUrl() == "https://www.example.com/index.html?key=value");

        REQUIRE_THROWS_CODE_MSG(UrlBuilder("https://www.+.com", handler),
                                ConnectionUrlSetupFailed,
                                "Curl URL error: Bad hostname");
    }
}
