// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSClientImpl.h"

#include "ErrorHandling.h"
#include "Logging.h"
#include "Responses.h"
#include "connection/Connection.h"
#include "connection/ConnectionManager.h"
#include "connection/CurlConnectionManager.h"
#include "connection/mock/MockConnectionManager.h"

using namespace SFS;
using namespace SFS::details;

constexpr const char* c_apiDomain = "api.cdp.microsoft.com";

template <typename ConnectionManagerT>
SFSClientImpl<ConnectionManagerT>::SFSClientImpl(ClientConfig&& config) : m_accountId(std::move(config.accountId))
{
    if (config.instanceId)
    {
        m_instanceId = std::move(*config.instanceId);
    }
    if (config.nameSpace)
    {
        m_nameSpace = std::move(*config.nameSpace);
    }
    if (config.logCallbackFn)
    {
        m_reportingHandler.SetLoggingCallback(std::move(*config.logCallbackFn));
    }

    static_assert(std::is_base_of<ConnectionManager, ConnectionManagerT>::value,
                  "ConnectionManagerT not derived from ConnectionManager");
    m_connectionManager = std::make_unique<ConnectionManagerT>(m_reportingHandler);
}

template <typename ConnectionManagerT>
Result SFSClientImpl<ConnectionManagerT>::GetLatestVersion(
    [[maybe_unused]] std::string_view productName,
    [[maybe_unused]] const std::optional<SearchAttributes>& attributes,
    [[maybe_unused]] Connection& connection,
    [[maybe_unused]] std::unique_ptr<VersionResponse>& response) const
{
    LOG_INFO(m_reportingHandler, "GetLatestVersion not implemented");
    return Result::E_NotImpl;
}

template <typename ConnectionManagerT>
Result SFSClientImpl<ConnectionManagerT>::GetSpecificVersion(
    [[maybe_unused]] std::string_view productName,
    [[maybe_unused]] std::string_view version,
    [[maybe_unused]] const std::optional<SearchAttributes>& attributes,
    [[maybe_unused]] Connection& connection,
    [[maybe_unused]] std::unique_ptr<VersionResponse>& content) const
{
    return Result::E_NotImpl;
}

template <typename ConnectionManagerT>
Result SFSClientImpl<ConnectionManagerT>::GetDownloadInfo(
    [[maybe_unused]] std::string_view productName,
    [[maybe_unused]] std::string_view version,
    [[maybe_unused]] const std::optional<SearchAttributes>& attributes,
    [[maybe_unused]] Connection& connection,
    [[maybe_unused]] std::unique_ptr<DownloadInfoResponse>& content) const
{
    return Result::E_NotImpl;
}

template <typename ConnectionManagerT>
ConnectionManager& SFSClientImpl<ConnectionManagerT>::GetConnectionManager()
{
    return *m_connectionManager;
}

template <typename ConnectionManagerT>
void SFSClientImpl<ConnectionManagerT>::SetCustomBaseUrl(std::string customBaseUrl)
{
    m_customBaseUrl = std::move(customBaseUrl);
}

template <typename ConnectionManagerT>
std::string SFSClientImpl<ConnectionManagerT>::GetBaseUrl() const
{
    if (m_customBaseUrl)
    {
        return *m_customBaseUrl;
    }
    return "https://" + m_accountId + "." + std::string(c_apiDomain);
}

template class SFS::details::SFSClientImpl<CurlConnectionManager>;
template class SFS::details::SFSClientImpl<MockConnectionManager>;
