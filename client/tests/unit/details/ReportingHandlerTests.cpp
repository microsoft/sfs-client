// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ReportingHandler.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>

#define TEST(...) TEST_CASE("[ReportingHandlerTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;

namespace
{
void WaitForCall(std::mutex& mutex, bool& called, std::condition_variable& cv)
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::seconds(1), [&called]() { return called; });
    REQUIRE(called);
    called = false;
}
} // namespace

TEST("Testing SetLoggingCallback()")
{
    ReportingHandler handler;

    std::mutex mutex;
    std::condition_variable cv;
    bool called = false;
    auto handling = [&](const LogData&) {
        std::lock_guard<std::mutex> guard(mutex);
        called = true;
        cv.notify_one();
    };

    handler.SetLoggingCallback(handling);
    REQUIRE_FALSE(called);

    LOG_INFO(handler, "Test");
    WaitForCall(mutex, called, cv);

    called = false;
    handler.SetLoggingCallback(nullptr);

    // No way to check if the callback was called or not, but it should not crash
    LOG_INFO(handler, "Test");
    REQUIRE_FALSE(called);
}

TEST("Testing Severities")
{
    ReportingHandler handler;

    std::optional<LogSeverity> severity;
    std::mutex mutex;
    std::condition_variable cv;
    bool called = false;
    auto handling = [&](const LogData& data) {
        std::lock_guard<std::mutex> guard(mutex);
        called = true;
        severity = data.severity;
        cv.notify_one();
    };

    handler.SetLoggingCallback(handling);

    REQUIRE(!severity.has_value());

    LOG_INFO(handler, "Test");
    WaitForCall(mutex, called, cv);

    REQUIRE(severity.has_value());
    REQUIRE(*severity == LogSeverity::Info);
    severity.reset();

    LOG_WARNING(handler, "Test");
    WaitForCall(mutex, called, cv);

    REQUIRE(severity.has_value());
    REQUIRE(*severity == LogSeverity::Warning);
    severity.reset();

    LOG_ERROR(handler, "Test");
    WaitForCall(mutex, called, cv);

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
    std::mutex mutex;
    std::condition_variable cv;
    bool called = false;
    auto handling = [&](const LogData& data) {
        std::lock_guard<std::mutex> guard(mutex);
        called = true;
        file = std::string(data.file);
        line = data.line;
        function = std::string(data.function);
        cv.notify_one();
    };

    handler.SetLoggingCallback(handling);

    LOG_INFO(handler, "Test");
    WaitForCall(mutex, called, cv);

    CHECK(file.find("ReportingHandlerTests.cpp") != std::string::npos);
    CHECK(line == (__LINE__ - 4));
    CHECK(function == "CATCH2_INTERNAL_TEST_4");

    LOG_WARNING(handler, "Test");
    WaitForCall(mutex, called, cv);

    CHECK(file.find("ReportingHandlerTests.cpp") != std::string::npos);
    CHECK(line == (__LINE__ - 4));
    CHECK(function == "CATCH2_INTERNAL_TEST_4");
}

TEST("Testing LogFormatting")
{
    ReportingHandler handler;

    std::string message;
    std::mutex mutex;
    std::condition_variable cv;
    bool called = false;
    auto handling = [&](const LogData& data) {
        std::lock_guard<std::mutex> guard(mutex);
        called = true;
        message = data.message;
        cv.notify_one();
    };

    handler.SetLoggingCallback(handling);

    REQUIRE(message.empty());

    LOG_INFO(handler, "Test %s", "Test");
    WaitForCall(mutex, called, cv);

    REQUIRE(message == "Test Test");

    LOG_WARNING(handler, "Test %s %s", "Test1", "Test2");
    WaitForCall(mutex, called, cv);

    REQUIRE(message == "Test Test1 Test2");

    LOG_ERROR(handler, "Test %s %s %s", "Test1", "Test2", "Test3");
    WaitForCall(mutex, called, cv);

    REQUIRE(message == "Test Test1 Test2 Test3");

    LOG_INFO(handler, "Test %d %d", 1, true);
    WaitForCall(mutex, called, cv);

    REQUIRE(message == "Test 1 1");

    LOG_INFO(handler, "Test %d %s", 2, false ? "true" : "false");
    WaitForCall(mutex, called, cv);

    REQUIRE(message == "Test 2 false");

    handler.SetLoggingCallback(nullptr);
}

TEST("Testing that SetLoggingCallback(nullptr) flushes existing messages with a counter")
{
    ReportingHandler handler;

    unsigned counter = 0;
    auto handling = [&](const LogData&) { ++counter; };
    handler.SetLoggingCallback(handling);

    unsigned maxCounter = 10000;
    for (unsigned i = 0; i < maxCounter; ++i)
    {
        LOG_INFO(handler, "Test %d", i);
    }

    unsigned counterBefore = counter;
    handler.SetLoggingCallback(nullptr);

    SECTION("Testing that all messages are flushed properly by waiting for the counter to reach maxCounter")
    {
        while (counter != maxCounter)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        INFO("Counter before: " << counterBefore << ", counter after: " << counter);
        CHECK(counterBefore < counter);
        REQUIRE(counter == maxCounter);
    }

    SECTION("Testing that setting a new callback handler has to wait until all messages are flushed")
    {
        auto newHandling = [&](const LogData&) {};
        handler.SetLoggingCallback(newHandling);

        INFO("Counter before: " << counterBefore << ", counter after SetLoggingCallback returns: " << counter);
        CHECK(counterBefore < counter);
        REQUIRE(counter == maxCounter);
    }

    SECTION("Testing that unsetting the callback handler another time has to wait until all messages are flushed")
    {
        handler.SetLoggingCallback(nullptr);

        INFO("Counter before: " << counterBefore << ", counter after SetLoggingCallback(nullptr) returns: " << counter);
        CHECK(counterBefore < counter);
        REQUIRE(counter == maxCounter);
    }
}

TEST("Testing ToString(LogSeverity)")
{
    REQUIRE(SFS::ToString(LogSeverity::Info) == "Info");
    REQUIRE(SFS::ToString(LogSeverity::Warning) == "Warning");
    REQUIRE(SFS::ToString(LogSeverity::Error) == "Error");
}
