// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ReportingHandler.h"

#include <chrono>

using namespace SFS;
using namespace SFS::details;

void ReportingHandler::SetLoggingCallback(LoggingCallbackFn&& callback)
{
    std::lock_guard<std::mutex> lock(m_loggingCallbackFnMutex);
    m_loggingCallbackFn = callback;
}

void ReportingHandler::LogWithSeverity(LogSeverity severity,
                                       const char* message,
                                       const char* file,
                                       int line,
                                       const char* function) const
{
    CallLoggingCallback(severity, message, file, line, function);
}

void ReportingHandler::CallLoggingCallback(LogSeverity severity,
                                           const char* message,
                                           const char* file,
                                           int line,
                                           const char* function) const
{
    std::lock_guard<std::mutex> lock(m_loggingCallbackFnMutex);
    if (m_loggingCallbackFn)
    {
        m_loggingCallbackFn(LogData{severity, message, file, line, function, std::chrono::system_clock::now()});
    }
}
