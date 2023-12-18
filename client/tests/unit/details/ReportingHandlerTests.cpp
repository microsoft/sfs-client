// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ReportingHandler.h"

#include <catch2/catch_test_macros.hpp>

#include <optional>

#define TEST(...) TEST_CASE("[ReportingHandlerTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;

TEST("Testing SetLoggingCallback()")
{
    ReportingHandler handler;

    bool called = false;
    auto handling = [&called](const LogData&) { called = true; };

    handler.SetLoggingCallback(handling);
    REQUIRE_FALSE(called);

    LOG_INFO(handler, "Test");

    REQUIRE(called);

    called = false;
    handler.SetLoggingCallback(nullptr);

    LOG_INFO(handler, "Test");
    REQUIRE_FALSE(called);
}

TEST("Testing Severities")
{
    ReportingHandler handler;

    std::optional<LogSeverity> severity;
    auto handling = [&severity](const LogData& data) { severity = data.severity; };
    handler.SetLoggingCallback(handling);

    REQUIRE(!severity.has_value());

    LOG_INFO(handler, "Test");
    REQUIRE(severity.has_value());
    REQUIRE(*severity == LogSeverity::Info);
    severity.reset();

    LOG_WARNING(handler, "Test");
    REQUIRE(severity.has_value());
    REQUIRE(*severity == LogSeverity::Warning);
    severity.reset();

    LOG_ERROR(handler, "Test");
    REQUIRE(severity.has_value());
    REQUIRE(*severity == LogSeverity::Error);

    handler.SetLoggingCallback(nullptr);
}

TEST("Testing file/line/function")
{
    ReportingHandler handler;

    std::string file;
    int line = 0;
    std::string function;
    auto handling = [&](const LogData& data) {
        file = std::string(data.file);
        line = data.line;
        function = std::string(data.function);
    };
    handler.SetLoggingCallback(handling);

    LOG_INFO(handler, "Test");
    CHECK(file.find("ReportingHandlerTests.cpp") != std::string::npos);
    CHECK(line == 77);
    CHECK(function == "CATCH2_INTERNAL_TEST_4");

    LOG_WARNING(handler, "Test");
    CHECK(file.find("ReportingHandlerTests.cpp") != std::string::npos);
    CHECK(line == 82);
    CHECK(function == "CATCH2_INTERNAL_TEST_4");
}

TEST("Testing LogFormatting")
{
    ReportingHandler handler;

    std::string message;
    auto handling = [&message](const LogData& data) { message = data.message; };
    handler.SetLoggingCallback(handling);

    REQUIRE(message.empty());

    LOG_INFO(handler, "Test %s", "Test");
    REQUIRE(message == "Test Test");

    LOG_WARNING(handler, "Test %s %s", "Test1", "Test2");
    REQUIRE(message == "Test Test1 Test2");

    LOG_ERROR(handler, "Test %s %s %s", "Test1", "Test2", "Test3");
    REQUIRE(message == "Test Test1 Test2 Test3");

    LOG_INFO(handler, "Test %d %d", 1, true);
    REQUIRE(message == "Test 1 1");

    LOG_INFO(handler, "Test %d %s", 2, false ? "true" : "false");
    REQUIRE(message == "Test 2 false");

    handler.SetLoggingCallback(nullptr);
}

TEST("Testing ToString(LogSeverity)")
{
    REQUIRE(SFS::ToString(LogSeverity::Info) == "Info");
    REQUIRE(SFS::ToString(LogSeverity::Warning) == "Warning");
    REQUIRE(SFS::ToString(LogSeverity::Error) == "Error");
}
