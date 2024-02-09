// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSClientImpl.h"

#include "ContentUtil.h"
#include "ErrorHandling.h"
#include "Logging.h"
#include "SFSUrlComponents.h"
#include "TestOverride.h"
#include "Util.h"
#include "connection/Connection.h"
#include "connection/ConnectionManager.h"
#include "connection/CurlConnectionManager.h"
#include "connection/mock/MockConnectionManager.h"

#include <nlohmann/json.hpp>

#include <unordered_set>

#define SFS_INFO(...) LOG_INFO(m_reportingHandler, __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;
using namespace SFS::details::contentutil;
using namespace SFS::details::util;
using json = nlohmann::json;

constexpr const char* c_apiDomain = "api.cdp.microsoft.com";
constexpr const char* c_defaultInstanceId = "default";
constexpr const char* c_defaultNameSpace = "default";

namespace
{
void LogIfTestOverridesAllowed(const ReportingHandler& handler)
{
    if (test::AreTestOverridesAllowed())
    {
        LOG_INFO(handler, "Test overrides are allowed");
    }
}

void ThrowInvalidResponseIfFalse(bool condition, const std::string& message, const ReportingHandler& handler)
{
    THROW_CODE_IF_LOG(ServiceInvalidResponse, !condition, handler, message);
}

json ParseServerMethodStringToJson(const std::string& data, const std::string& method, const ReportingHandler& handler)
{
    try
    {
        return json::parse(data);
    }
    catch (json::parse_error& ex)
    {
        THROW_LOG(
            Result(Result::ServiceInvalidResponse, "(" + method + ") JSON Parsing error: " + std::string(ex.what())),
            handler);
    }
}

std::vector<ContentId> GetLatestVersionBatchResponseToContentIds(const json& data, const ReportingHandler& handler)
{
    // Expected format:
    // [
    //   {
    //     "ContentId": {
    //       "Namespace": <ns>,
    //       "Name": <name>,
    //       "Version": <version>
    //     }
    //   },
    //   ...
    // ]
    //

    ThrowInvalidResponseIfFalse(data.is_array(), "Response is not a JSON array", handler);
    ThrowInvalidResponseIfFalse(data.size() > 0, "Response does not have the expected size", handler);

    std::vector<ContentId> contentIds;
    for (const auto& obj : data)
    {
        ThrowInvalidResponseIfFalse(obj.is_object(), "Array element is not a JSON object", handler);
        ThrowInvalidResponseIfFalse(obj.contains("ContentId"), "Missing ContentId in response", handler);
        contentIds.push_back(std::move(*ContentIdJsonToObj(obj["ContentId"], handler)));
    }

    return contentIds;
}

std::unique_ptr<ContentId> GetSpecificVersionResponseToContentId(const json& data, const ReportingHandler& handler)
{
    // Expected format:
    // {
    //   "ContentId": {
    //     "Namespace": <ns>,
    //     "Name": <name>,
    //     "Version": <version>
    //   },
    //   "Files": [
    //     <file1>,
    //     ...
    //   ]
    // }
    //
    // We don't care about Files in this response, so we just ignore them

    ThrowInvalidResponseIfFalse(data.is_object(), "Response is not a JSON object", handler);

    return ContentIdJsonToObj(data["ContentId"], handler);
}

std::vector<File> GetDownloadInfoResponseToFileVector(const json& data, const ReportingHandler& handler)
{
    // Expected format:
    // [
    //   {
    //     "FileId": <fileid>,
    //     "Url": <url>,
    //     "SizeInBytes": <size>,
    //     "Hashes": {
    //       "Sha1": <sha1>,
    //       "Sha256": <sha2>
    //     },
    //     "DeliveryOptimization": {
    //       "CatalogId": <catalogid>,
    //       "Properties": {
    //         <pair_value_list>
    //       }
    //     }
    //   },
    //   ...
    // ]

    ThrowInvalidResponseIfFalse(data.is_array(), "Response is not a JSON array", handler);

    // TODO #48: For now ignore DeliveryOptimization data. Will implement its separate parsing later

    std::vector<File> tmp;
    for (const auto& fileData : data)
    {
        ThrowInvalidResponseIfFalse(fileData.is_object(), "Array element is not a JSON object", handler);
        tmp.push_back(std::move(*FileJsonToObj(fileData, handler)));
    }

    return tmp;
}

bool DoesGetVersionResponseMatchProduct(const ContentId& contentId, std::string_view nameSpace, std::string_view name)
{
    return AreEqualI(contentId.GetNameSpace(), nameSpace) && AreEqualI(contentId.GetName(), name);
}
} // namespace

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

    LogIfTestOverridesAllowed(m_reportingHandler);
}

template <typename ConnectionManagerT>
std::vector<ContentId> SFSClientImpl<ConnectionManagerT>::GetLatestVersionBatch(
    const std::vector<ProductRequest> productRequests,
    Connection& connection) const
try
{
    const std::string url{SFSUrlComponents::GetLatestVersionBatchUrl(GetBaseUrl(), m_instanceId, m_nameSpace)};

    SFS_INFO("Requesting latest version of multiple products from URL [%s]", url.c_str());

    // Creating request body
    std::unordered_set<std::string> productNames;
    json body = json::array();
    for (const auto& [productName, attributes] : productRequests)
    {
        SFS_INFO("Product #%zu: [%s]", body.size() + size_t{1}, productName.c_str());
        productNames.insert(productName);

        json targettingAttributes = json::object();
        for (const auto& [key, value] : attributes)
        {
            targettingAttributes[key] = value;
        }
        body.push_back({{"TargetingAttributes", targettingAttributes}, {"Product", productName}});
    }

    SFS_INFO("Request body [%s]", body.dump().c_str());

    const std::string postResponse{connection.Post(url, body.dump())};

    const json versionResponse =
        ParseServerMethodStringToJson(postResponse, "GetLatestVersionBatch", m_reportingHandler);

    auto contentIds = GetLatestVersionBatchResponseToContentIds(versionResponse, m_reportingHandler);

    // Validating responses
    for (const auto& contentId : contentIds)
    {
        THROW_CODE_IF_LOG(ServiceInvalidResponse,
                          productNames.count(contentId.GetName()) == 0,
                          m_reportingHandler,
                          "(GetLatestVersionBatch) Response of product [" + contentId.GetName() +
                              "] does not match the requested products");
        THROW_CODE_IF_LOG(ServiceInvalidResponse,
                          AreNotEqualI(contentId.GetNameSpace(), m_nameSpace),
                          m_reportingHandler,
                          "(GetLatestVersionBatch) Response of product [" + contentId.GetName() +
                              "] does not match the requested namespace");

        SFS_INFO("Received a response for product [%s] with version %s",
                 contentId.GetName().c_str(),
                 contentId.GetVersion().c_str());
    }

    return contentIds;
}
SFS_CATCH_LOG_RETHROW(m_reportingHandler)

template <typename ConnectionManagerT>
std::unique_ptr<ContentId> SFSClientImpl<ConnectionManagerT>::GetSpecificVersion(const std::string& productName,
                                                                                 const std::string& version,
                                                                                 Connection& connection) const
try
{
    const std::string url{
        SFSUrlComponents::GetSpecificVersionUrl(GetBaseUrl(), m_instanceId, m_nameSpace, productName, version)};

    SFS_INFO("Requesting version [%s] of [%s] from URL [%s]", version.c_str(), productName.c_str(), url.c_str());

    const std::string getResponse{connection.Get(url)};

    const json versionResponse = ParseServerMethodStringToJson(getResponse, "GetSpecificVersion", m_reportingHandler);

    auto contentId = GetSpecificVersionResponseToContentId(versionResponse, m_reportingHandler);
    THROW_CODE_IF_LOG(ServiceInvalidResponse,
                      !DoesGetVersionResponseMatchProduct(*contentId, m_nameSpace, productName),
                      m_reportingHandler,
                      "(GetSpecificVersion) Response does not match the requested product");

    SFS_INFO("Received the expected response with version %s", contentId->GetVersion().c_str());

    return contentId;
}
SFS_CATCH_LOG_RETHROW(m_reportingHandler)

template <typename ConnectionManagerT>
std::vector<File> SFSClientImpl<ConnectionManagerT>::GetDownloadInfo(const std::string& productName,
                                                                     const std::string& version,
                                                                     Connection& connection) const
try
{
    const std::string url{
        SFSUrlComponents::GetDownloadInfoUrl(GetBaseUrl(), m_instanceId, m_nameSpace, productName, version)};

    SFS_INFO("Requesting download info of version [%s] of [%s] from URL [%s]",
             version.c_str(),
             productName.c_str(),
             url.c_str());

    const std::string postResponse{connection.Post(url)};

    const json downloadInfoResponse =
        ParseServerMethodStringToJson(postResponse, "GetDownloadInfo", m_reportingHandler);

    auto files = GetDownloadInfoResponseToFileVector(downloadInfoResponse, m_reportingHandler);

    SFS_INFO("Received a response with %zu files", files.size());

    return files;
}
SFS_CATCH_LOG_RETHROW(m_reportingHandler)

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
    if (auto envVar = test::GetTestOverride(test::TestOverride::BaseUrl))
    {
        return *envVar;
    }

    if (m_customBaseUrl)
    {
        return *m_customBaseUrl;
    }
    return "https://" + m_accountId + "." + std::string(c_apiDomain);
}

template class SFS::details::SFSClientImpl<CurlConnectionManager>;
template class SFS::details::SFSClientImpl<MockConnectionManager>;
