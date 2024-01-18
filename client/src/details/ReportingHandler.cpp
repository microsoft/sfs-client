// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ReportingHandler.h"

#include <chrono>

using namespace SFS;
using namespace SFS::details;

ReportingHandler::ReportingHandler()
{
}

ReportingHandler::~ReportingHandler()
{
    StopLoggingThread();
}

void ReportingHandler::SetLoggingCallback(LoggingCallbackFn&& callback)
{
    // Wait if we are shutting down so we don't set a new callback while another is being destroyed
    {
        std::lock_guard<std::mutex> shuttingDownGuard(m_threadShuttingDownMutex);
    }

    // If unsetting, first flush all logs with the existing callback and stop the logging thread
    if (!callback)
    {
        StopLoggingThread();
    }

    std::lock_guard<std::mutex> guard(m_loggingCallbackFnMutex);
    m_loggingCallbackFn = callback;
    if (m_loggingCallbackFn)
    {
        StartLoggingThread();
    }
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
    // Don't queue if there is no callback set
    {
        std::lock_guard<std::mutex> guard(m_loggingCallbackFnMutex);
        if (!m_loggingCallbackFn)
        {
            return;
        }
    }

    std::lock_guard<std::mutex> guard(m_threadMutex);

    // Don't queue anymore if we are shutting down
    if (m_threadShutdown)
    {
        return;
    }

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
    bool running = true;
    while (running)
    {
        {
            // Wait for a signal
            std::unique_lock<std::mutex> lock(m_threadMutex);
            m_threadCondition.wait(lock, [this]() { return !m_logDataQueue.empty() || m_threadShutdown; });
        }

        FlushLastLog();

        // Shutdown if requested
        {
            std::lock_guard<std::mutex> guard(m_threadMutex);
            running = !m_threadShutdown;
        }
    }

    // Flush remaining logs if needed, now there should be no more additions to the queue
    while (!m_logDataQueue.empty())
    {
        FlushLastLog();
    }
}

void ReportingHandler::FlushLastLog()
{
    // Get last log data
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
}

void ReportingHandler::StartLoggingThread()
{
    if (!m_loggingThread.joinable())
    {
        m_threadShutdown = false;
        m_loggingThread = std::thread(&ReportingHandler::ProcessLogging, this);
    }
}

void ReportingHandler::StopLoggingThread()
{
    std::lock_guard<std::mutex> shuttingDownGuard(m_threadShuttingDownMutex);
    if (m_loggingThread.joinable())
    {
        {
            std::lock_guard<std::mutex> guard(m_threadMutex);
            m_threadShutdown = true;
        }
        m_threadCondition.notify_one();
        m_loggingThread.join();
    }
}
