// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ConnectionManager.h"

#include "../ErrorHandling.h"
#include "Connection.h"

#include <curl/curl.h>

using namespace SFS::details;

ConnectionManager::ConnectionManager(const ReportingHandler& handler) : m_handler(handler)
{
}

CurlConnectionManager::CurlConnectionManager(const ReportingHandler& handler) : ConnectionManager(handler)
{
    auto result = curl_global_init(CURL_GLOBAL_ALL);
    if (result != CURLE_OK)
    {
        throw SFSException(Result(Result::E_Unexpected, "Curl failed to initialize"));
    }
}

CurlConnectionManager::~CurlConnectionManager()
{
    curl_global_cleanup();
}

std::unique_ptr<Connection> CurlConnectionManager::MakeConnection()
{
    return std::make_unique<CurlConnection>(m_handler);
}
