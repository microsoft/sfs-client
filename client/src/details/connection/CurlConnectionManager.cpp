// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "CurlConnectionManager.h"

#include "../ErrorHandling.h"
#include "CurlConnection.h"

#include <curl/curl.h>

using namespace SFS;
using namespace SFS::details;

namespace
{
// Curl recommends checking for expected features in runtime
Result CheckCurlFeatures(const ReportingHandler& handler)
{
    curl_version_info_data* ver = curl_version_info(CURLVERSION_NOW);
    RETURN_CODE_IF_LOG(HttpUnexpected, !ver, handler);

    RETURN_CODE_IF_LOG(HttpUnexpected, !(ver->features & CURL_VERSION_SSL), handler, "Curl was not built with SSL");
    RETURN_CODE_IF_LOG(HttpUnexpected, !(ver->features & CURL_VERSION_THREADSAFE), handler, "Curl is not thread safe");

    // For thread safety we need the DNS resolutions to be asynchronous (which happens because of c-ares)
    RETURN_CODE_IF_LOG(HttpUnexpected,
                       !(ver->features & CURL_VERSION_ASYNCHDNS),
                       handler,
                       "Curl was not built with async DNS resolutions");

    return Result::Success;
}
} // namespace

Result CurlConnectionManager::Make(const ReportingHandler& handler, std::unique_ptr<ConnectionManager>& out)
{
    auto tmp = std::unique_ptr<CurlConnectionManager>(new CurlConnectionManager(handler));
    RETURN_IF_FAILED_LOG(tmp->SetupCurl(), handler);
    out = std::move(tmp);

    return Result::Success;
}

CurlConnectionManager::CurlConnectionManager(const ReportingHandler& handler) : ConnectionManager(handler)
{
}

CurlConnectionManager::~CurlConnectionManager()
{
    curl_global_cleanup();
}

Result CurlConnectionManager::SetupCurl()
{
    RETURN_CODE_IF_LOG(HttpUnexpected,
                       curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK,
                       m_handler,
                       "Curl failed to initialize");
    RETURN_IF_FAILED_LOG(CheckCurlFeatures(m_handler), m_handler);
    return Result::Success;
}

Result CurlConnectionManager::MakeConnection(std::unique_ptr<Connection>& out)
{
    RETURN_IF_FAILED_LOG(CurlConnection::Make(m_handler, out), m_handler);
    return Result::Success;
}
