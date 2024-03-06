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
        return json(); // Unreachable code, but the compiler doesn't know that.
    }
}

VersionEntities ConvertLatestVersionBatchResponseToVersionEntities(const json& data, const ReportingHandler& handler)
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

    VersionEntities entities;
    for (const auto& obj : data)
    {
        entities.push_back(std::move(ParseJsonToVersionEntity(obj, handler)));
    }

    return entities;
}

std::vector<File> ConvertDownloadInfoResponseToFileVector(const json& data, const ReportingHandler& handler)
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

bool VerifyVersionResponseMatchesProduct(const ContentIdEntity& contentId,
                                         std::string_view nameSpace,
                                         std::string_view name)
{
    return contentId.nameSpace == nameSpace && contentId.name == name;
}

void ValidateVersionEntity(const VersionEntity& versionEntity,
                           const std::string& nameSpace,
                           const std::string& product,
                           const ReportingHandler& handler)
{
    THROW_CODE_IF_NOT_LOG(ServiceInvalidResponse,
                          VerifyVersionResponseMatchesProduct(versionEntity.contentId, nameSpace, product),
                          handler,
                          "Response does not match the requested product");
}

void ValidateBatchVersionEntity(const VersionEntities& versionEntities,
                                const std::string& nameSpace,
                                const std::unordered_set<std::string>& requestedProducts,
                                const ReportingHandler& handler)
{
    for (const auto& entity : versionEntities)
    {
        THROW_CODE_IF_LOG(ServiceInvalidResponse,
                          requestedProducts.count(entity->contentId.name) == 0,
                          handler,
                          "Received product [" + entity->contentId.name +
                              "] which is not one of the requested products");
        THROW_CODE_IF_LOG(ServiceInvalidResponse,
                          AreNotEqualI(entity->contentId.nameSpace, nameSpace),
                          handler,
                          "Received product [" + entity->contentId.name + "] with a namespace [" +
                              entity->contentId.nameSpace + "] that does not match the requested namespace");

        LOG_INFO(handler,
                 "Received a response for product [%s] with version %s",
                 entity->contentId.name.c_str(),
                 entity->contentId.version.c_str());
    }
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
std::unique_ptr<VersionEntity> SFSClientImpl<ConnectionManagerT>::GetLatestVersion(const ProductRequest& productRequest,
                                                                                   Connection& connection) const
try
{
    const auto& [product, attributes] = productRequest;
    const std::string url{SFSUrlComponents::GetLatestVersionUrl(GetBaseUrl(), m_instanceId, m_nameSpace, product)};

    LOG_INFO(m_reportingHandler, "Requesting latest version of [%s] from URL [%s]", product.c_str(), url.c_str());

    const json body = {{"TargetingAttributes", attributes}};
    LOG_VERBOSE(m_reportingHandler, "Request body [%s]", body.dump().c_str());

    const std::string postResponse{connection.Post(url, body.dump())};
    const json versionResponse = ParseServerMethodStringToJson(postResponse, "GetLatestVersion", m_reportingHandler);

    auto versionEntity = ParseJsonToVersionEntity(versionResponse, m_reportingHandler);
    ValidateVersionEntity(*versionEntity, m_nameSpace, product, m_reportingHandler);

    LOG_INFO(m_reportingHandler, "Received a response with version %s", versionEntity->contentId.version.c_str());

    return versionEntity;
}
SFS_CATCH_LOG_RETHROW(m_reportingHandler)

template <typename ConnectionManagerT>
VersionEntities SFSClientImpl<ConnectionManagerT>::GetLatestVersionBatch(
    const std::vector<ProductRequest>& productRequests,
    Connection& connection) const
try
{
    const std::string url{SFSUrlComponents::GetLatestVersionBatchUrl(GetBaseUrl(), m_instanceId, m_nameSpace)};

    LOG_INFO(m_reportingHandler, "Requesting latest version of multiple products from URL [%s]", url.c_str());

    // Creating request body
    std::unordered_set<std::string> requestedProducts;
    json body = json::array();
    for (const auto& [product, attributes] : productRequests)
    {
        LOG_INFO(m_reportingHandler, "Product #%zu: [%s]", body.size() + size_t{1}, product.c_str());
        requestedProducts.insert(product);

        body.push_back({{"TargetingAttributes", attributes}, {"Product", product}});
    }

    LOG_VERBOSE(m_reportingHandler, "Request body [%s]", body.dump().c_str());

    const std::string postResponse{connection.Post(url, body.dump())};

    const json versionResponse =
        ParseServerMethodStringToJson(postResponse, "GetLatestVersionBatch", m_reportingHandler);

    auto entities = ConvertLatestVersionBatchResponseToVersionEntities(versionResponse, m_reportingHandler);
    ValidateBatchVersionEntity(entities, m_nameSpace, requestedProducts, m_reportingHandler);

    return entities;
}
SFS_CATCH_LOG_RETHROW(m_reportingHandler)

template <typename ConnectionManagerT>
std::unique_ptr<VersionEntity> SFSClientImpl<ConnectionManagerT>::GetSpecificVersion(const std::string& product,
                                                                                     const std::string& version,
                                                                                     Connection& connection) const
try
{
    const std::string url{
        SFSUrlComponents::GetSpecificVersionUrl(GetBaseUrl(), m_instanceId, m_nameSpace, product, version)};

    LOG_INFO(m_reportingHandler,
             "Requesting version [%s] of [%s] from URL [%s]",
             version.c_str(),
             product.c_str(),
             url.c_str());

    const std::string getResponse{connection.Get(url)};

    const json versionResponse = ParseServerMethodStringToJson(getResponse, "GetSpecificVersion", m_reportingHandler);

    auto versionEntity = ParseJsonToVersionEntity(versionResponse, m_reportingHandler);
    ValidateVersionEntity(*versionEntity, m_nameSpace, product, m_reportingHandler);

    LOG_INFO(m_reportingHandler,
             "Received the expected response with version %s",
             versionEntity->contentId.version.c_str());

    return versionEntity;
}
SFS_CATCH_LOG_RETHROW(m_reportingHandler)

template <typename ConnectionManagerT>
std::vector<File> SFSClientImpl<ConnectionManagerT>::GetDownloadInfo(const std::string& product,
                                                                     const std::string& version,
                                                                     Connection& connection) const
try
{
    const std::string url{
        SFSUrlComponents::GetDownloadInfoUrl(GetBaseUrl(), m_instanceId, m_nameSpace, product, version)};

    LOG_INFO(m_reportingHandler,
             "Requesting download info of version [%s] of [%s] from URL [%s]",
             version.c_str(),
             product.c_str(),
             url.c_str());

    const std::string postResponse{connection.Post(url)};

    const json downloadInfoResponse =
        ParseServerMethodStringToJson(postResponse, "GetDownloadInfo", m_reportingHandler);

    auto files = ConvertDownloadInfoResponseToFileVector(downloadInfoResponse, m_reportingHandler);

    LOG_INFO(m_reportingHandler, "Received a response with %zu files", files.size());

    return files;
}
SFS_CATCH_LOG_RETHROW(m_reportingHandler)

template <typename ConnectionManagerT>
std::unique_ptr<Connection> SFSClientImpl<ConnectionManagerT>::MakeConnection(const ConnectionConfig& config)
{
    return m_connectionManager->MakeConnection(config);
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
