// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSClientImpl.h"

#include "ErrorHandling.h"
#include "Logging.h"
#include "Responses.h"
#include "SFSUrlComponents.h"
#include "connection/Connection.h"
#include "connection/ConnectionManager.h"
#include "connection/CurlConnectionManager.h"
#include "connection/mock/MockConnectionManager.h"

#include <nlohmann/json.hpp>

#define SFS_INFO(...) LOG_INFO(m_reportingHandler, __VA_ARGS__)
#define SFS_RETURN_IF_FAILED(result) RETURN_IF_FAILED_LOG(result, m_reportingHandler)

using namespace SFS;
using namespace SFS::details;
using json = nlohmann::json;

constexpr const char* c_apiDomain = "api.cdp.microsoft.com";
constexpr const char* c_defaultInstanceId = "default";
constexpr const char* c_defaultNameSpace = "default";

template <typename ConnectionManagerT>
SFSClientImpl<ConnectionManagerT>::SFSClientImpl(ClientConfig&& config)
    : m_accountId(std::move(config.accountId))
    , m_instanceId(config.instanceId && !config.instanceId->empty() ? std::move(*config.instanceId)
                                                                    : c_defaultInstanceId)
    , m_nameSpace(config.nameSpace && !config.nameSpace->empty() ? std::move(*config.nameSpace) : c_defaultNameSpace)
{
    if (config.logCallbackFn)
    {
        m_reportingHandler.SetLoggingCallback(std::move(*config.logCallbackFn));
    }

    static_assert(std::is_base_of<ConnectionManager, ConnectionManagerT>::value,
                  "ConnectionManagerT not derived from ConnectionManager");
    m_connectionManager = std::make_unique<ConnectionManagerT>(m_reportingHandler);
}

template <typename ConnectionManagerT>
Result SFSClientImpl<ConnectionManagerT>::GetLatestVersion(const std::string& productName,
                                                           const std::optional<SearchAttributes>& attributes,
                                                           Connection& connection,
                                                           std::unique_ptr<VersionResponse>& response) const
{
    const std::string url{SFSUrlComponents::GetLatestVersionUrl(GetBaseUrl(), m_instanceId, m_nameSpace)};

    SFS_INFO("Requesting latest version of [%s] from URL [%s]", productName.c_str(), url.c_str());

    json targettingAttributes = json::object();
    for (const auto& [key, value] : attributes.value_or(SearchAttributes()))
    {
        targettingAttributes[key] = value;
    }
    json body = json::array();
    body.push_back({{"TargetingAttributes", targettingAttributes}, {"Product", productName}});

    SFS_INFO("Request body [%s]", body.dump().c_str());

    std::string out;
    SFS_RETURN_IF_FAILED(connection.Post(url, body.dump(), out));

    // TODO: currently the response is just being sent as is, we have to parse it and check the return values
    // TODO: Check for json::parse exceptions
    response = std::make_unique<VersionResponse>(json::parse(out));

    return Result::Success;
}

template <typename ConnectionManagerT>
Result SFSClientImpl<ConnectionManagerT>::GetSpecificVersion(const std::string& productName,
                                                             const std::string& version,
                                                             Connection& connection,
                                                             std::unique_ptr<VersionResponse>& response) const
{
    const std::string url{
        SFSUrlComponents::GetSpecificVersionUrl(GetBaseUrl(), m_instanceId, m_nameSpace, productName, version)};

    SFS_INFO("Requesting version [%s] of [%s] from URL [%s]", version.c_str(), productName.c_str(), url.c_str());

    std::string out;
    SFS_RETURN_IF_FAILED(connection.Get(url, out));

    // TODO: currently the response is just being sent as is, we have to parse it and check the return values
    // TODO: Check for json::parse exceptions
    response = std::make_unique<VersionResponse>(json::parse(out));

    return Result::Success;
}

template <typename ConnectionManagerT>
Result SFSClientImpl<ConnectionManagerT>::GetDownloadInfo(const std::string& productName,
                                                          const std::string& version,
                                                          Connection& connection,
                                                          std::unique_ptr<DownloadInfoResponse>& response) const
{
    const std::string url{
        SFSUrlComponents::GetDownloadInfoUrl(GetBaseUrl(), m_instanceId, m_nameSpace, productName, version)};

    SFS_INFO("Requesting download info of version [%s] of [%s] from URL [%s]",
             version.c_str(),
             productName.c_str(),
             url.c_str());

    std::string out;
    SFS_RETURN_IF_FAILED(connection.Post(url, out));

    // TODO: currently the response is just being sent as is, we have to parse it and check the return values
    // TODO: Check for json::parse exceptions
    response = std::make_unique<DownloadInfoResponse>(json::parse(out));

    return Result::Success;
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
