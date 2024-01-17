// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ReportingHandler.h"

#include <chrono>

using namespace SFS;
using namespace SFS::details;

ReportingHandler::ReportingHandler()
{
    m_loggingThread = std::thread(&ReportingHandler::ProcessLogging, this);
}

ReportingHandler::~ReportingHandler()
{
    StopLoggingThread();
}

void ReportingHandler::SetLoggingCallback(LoggingCallbackFn&& callback)
{
    std::lock_guard<std::mutex> guard(m_loggingCallbackFnMutex);
    m_loggingCallbackFn = callback;
}

void ReportingHandler::LogWithSeverity(LogSeverity severity,
                                       const char* message,
                                       const char* file,
                                       unsigned line,
                                       const char* function) const
{
    QueueLogData(severity, message, file, line, function);
}

void ReportingHandler::QueueLogData(LogSeverity severity,
                                    const char* message,
                                    const char* file,
                                    unsigned line,
                                    const char* function) const
{
    // Don't queue anymore if we are shutting down
    if (m_threadShutdown)
    {
        return;
    }

    std::lock_guard<std::mutex> guard(m_threadMutex);
    try
    {
        m_logDataQueue.push({severity, message, file, line, function, std::chrono::system_clock::now()});
        m_threadCondition.notify_one();
    }
    catch (...)
    {
        // Ignore exceptions, lose callback message
    }
}

void ReportingHandler::ProcessLogging()
{
    while (true)
    {
        {
            // Wait for a signal
            std::unique_lock<std::mutex> lock(m_threadMutex);
            m_threadCondition.wait(lock, [this]() { return !m_logDataQueue.empty() || m_threadShutdown; });
        }

        FlushLogs();

        // Shutdown if requested
        {
            std::lock_guard<std::mutex> guard(m_threadMutex);
            if (m_threadShutdown)
            {
                break;
            }
        }
    }
}

void ReportingHandler::FlushLogs()
{
    bool hasMoreLogData = false;
    do
    {
        // Get next log data
        LogData logData;
        {
            std::lock_guard<std::mutex> guard(m_threadMutex);
            if (m_logDataQueue.empty())
            {
                return;
            }

            logData = m_logDataQueue.front();
            m_logDataQueue.pop();
        }

        // Call logging callback
        {
            std::lock_guard<std::mutex> guard(m_loggingCallbackFnMutex);
            if (m_loggingCallbackFn)
            {
                m_loggingCallbackFn(logData);
            }
        }

        // Check if there is more log data
        {
            std::lock_guard<std::mutex> guard(m_threadMutex);
            hasMoreLogData = !m_logDataQueue.empty();
        }
    } while (hasMoreLogData);
}

void ReportingHandler::StopLoggingThread()
{
    {
        std::lock_guard<std::mutex> guard(m_threadMutex);
        m_threadShutdown = true;
    }
    m_threadCondition.notify_one();
    if (m_loggingThread.joinable())
    {
        m_loggingThread.join();
    }
}
