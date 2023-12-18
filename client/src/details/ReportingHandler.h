// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Logging.h"

#include <mutex>
#include <stdio.h>

#define MAX_LOG_MESSAGE_SIZE 1024

#define LOG_SEVERITY(handler, severity, format, ...)                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        char __message[MAX_LOG_MESSAGE_SIZE];                                                                          \
        snprintf(__message, MAX_LOG_MESSAGE_SIZE, format, ##__VA_ARGS__);                                              \
        handler.LogWithSeverity(severity, __message, __FILE__, __LINE__, __FUNCTION__);                                \
    } while ((void)0, 0)

#define LOG_INFO(handler, format, ...) LOG_SEVERITY(handler, SFS::LogSeverity::Info, format, ##__VA_ARGS__)
#define LOG_WARNING(handler, format, ...) LOG_SEVERITY(handler, SFS::LogSeverity::Warning, format, ##__VA_ARGS__)
#define LOG_ERROR(handler, format, ...) LOG_SEVERITY(handler, SFS::LogSeverity::Error, format, ##__VA_ARGS__)

namespace SFS::details
{
/**
 * @brief This class enables thread-safe access to the externally set logging callback function.
 * @details Each SFSClient instance will have one ReportingHandler instance, and access to the logging callback function
 * is controlled by a mutex that makes sure that only one thread can access the logging callback function at a time.
 */
class ReportingHandler
{
  public:
    ReportingHandler() = default;

    ReportingHandler(const ReportingHandler&) = delete;
    ReportingHandler& operator=(const ReportingHandler&) = delete;

    /**
     * @brief Sets the logging callback function.
     * @details This function is thread-safe.
     * @param callback The logging callback function. To reset, pass a nullptr.
     */
    void SetLoggingCallback(LoggingCallbackFn&& callback);

    /**
     * @brief Logs a message with the given severity.
     * @details Prefer calling it with macros LOG_INFO, LOG_WARNING, LOG_ERROR so file, line and function are
     * automatically populated and the message can be formatted.
     */
    void LogWithSeverity(LogSeverity severity,
                         const char* message,
                         const char* file,
                         int line,
                         const char* function) const;

  private:
    void CallLoggingCallback(LogSeverity severity,
                             const char* message,
                             const char* file,
                             int line,
                             const char* function) const;

    LoggingCallbackFn m_loggingCallbackFn;
    mutable std::mutex m_loggingCallbackFnMutex;
};
} // namespace SFS::details