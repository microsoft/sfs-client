// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Logging.h"

#include <mutex>
#include <sstream>

#include <stdio.h>

#define MAX_LOG_MESSAGE_SIZE 1024

#define LOG_INFO_STM(handler, severity, format)                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        MessageStream stm(handler, severity, __FILE__, __LINE__, __FUNCTION__);                                        \
        stm << format;                                                                                                 \
    } while ((void)0, 0)

#define LOG_INFO(handler, format) LOG_INFO_STM(handler, SFS::LogSeverity::Info, format)
#define LOG_WARNING(handler, format) LOG_INFO_STM(handler, SFS::LogSeverity::Warning, format)
#define LOG_ERROR(handler, format) LOG_INFO_STM(handler, SFS::LogSeverity::Error, format)
#define LOG_VERBOSE(handler, format) LOG_INFO_STM(handler, SFS::LogSeverity::Verbose, format)

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
     * @details Prefer calling it with macros LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_VERBOSE so file, line and function
     * are automatically populated and the message can be formatted.
     */
    void LogWithSeverity(LogSeverity severity,
                         const char* file,
                         unsigned line,
                         const char* function,
                         const char* message) const
    {
        CallLoggingCallback(severity, message, file, line, function);
    }

    /**
     * @brief Logs a message with the given severity.
     * @details Prefer calling it with macros LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_VERBOSE so file, line and function
     * are automatically populated and the message can be formatted.
     */
    template <typename... Args>
    void LogWithSeverity(LogSeverity severity,
                         const char* file,
                         unsigned line,
                         const char* function,
                         const char* format,
                         const Args&... args) const
    {
        constexpr std::size_t n = sizeof...(Args);
        if constexpr (n == 0)
        {
            CallLoggingCallback(severity, format, file, line, function);
        }
        else
        {
            char message[MAX_LOG_MESSAGE_SIZE];
            snprintf(message, MAX_LOG_MESSAGE_SIZE, format, args...);
            CallLoggingCallback(severity, message, file, line, function);
        }
    }

  private:
    void CallLoggingCallback(LogSeverity severity,
                             const char* message,
                             const char* file,
                             unsigned line,
                             const char* function) const;

    LoggingCallbackFn m_loggingCallbackFn;
    mutable std::mutex m_loggingCallbackFnMutex;
};

/**
 * @brief This class implements an ostream for the LOG macros.
 * @details Lifetime of the arguments must as long as the lifetime of this object.
 */
class MessageStream
{
  public:
    explicit MessageStream(const ReportingHandler& handler,
                           LogSeverity severity,
                           const char* file,
                           unsigned int line,
                           const char* function)
        : m_handler(handler)
        , m_severity(severity)
        , m_file(file)
        , m_line(line)
        , m_function(function)
    {
    }

    ~MessageStream()
    {
        m_handler.LogWithSeverity(m_severity, m_file, m_line, m_function, m_stream.str().c_str());
    }

    template <typename T>
    MessageStream& operator<<(const T& value)
    {
        m_stream << value;
        return *this;
    }

  private:
    const ReportingHandler& m_handler;
    const LogSeverity m_severity;
    const char* m_file;
    unsigned int m_line;
    const char* m_function;

    std::ostringstream m_stream;
};

} // namespace SFS::details
