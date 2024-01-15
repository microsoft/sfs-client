// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <chrono>
#include <functional>
#include <string>

namespace SFS
{
enum LogSeverity
{
    Info,
    Warning,
    Error
};

struct LogData
{
    LogSeverity severity;
    std::string message;
    std::string file;
    int line;
    std::string function;
    std::chrono::time_point<std::chrono::system_clock> time;
};

using LoggingCallbackFn = std::function<void(const LogData&)>;

std::string_view ToString(LogSeverity severity) noexcept;
}; // namespace SFS
