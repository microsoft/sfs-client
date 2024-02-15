// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSClientImpl.h"

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

#define SFS_INFO(...) LOG_INFO(m_reportingHandler, __VA_ARGS__)
#define THROW_INVALID_RESPONSE_IF_FALSE(condition, message, handler)                                                   \
    THROW_CODE_IF_LOG(ServiceInvalidResponse, !(condition), handler, message)

using namespace SFS;
using namespace SFS::details;
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

std::unique_ptr<ContentId> ContentIdJsonToObj(const json& contentId, const ReportingHandler& handler)
{
    THROW_INVALID_RESPONSE_IF_FALSE(contentId.is_object(), "ContentId is not a JSON object", handler);

    THROW_INVALID_RESPONSE_IF_FALSE(contentId.contains("Namespace"),
                                    "Missing ContentId.Namespace in response",
                                    handler);
    THROW_INVALID_RESPONSE_IF_FALSE(contentId["Namespace"].is_string(), "ContentId.Namespace is not a string", handler);
    std::string nameSpace = contentId["Namespace"];

    THROW_INVALID_RESPONSE_IF_FALSE(contentId.contains("Name"), "Missing ContentId.Name in response", handler);
    THROW_INVALID_RESPONSE_IF_FALSE(contentId["Name"].is_string(), "ContentId.Name is not a string", handler);
    std::string name = contentId["Name"];

    THROW_INVALID_RESPONSE_IF_FALSE(contentId.contains("Version"), "Missing ContentId.Version in response", handler);
    THROW_INVALID_RESPONSE_IF_FALSE(contentId["Version"].is_string(), "ContentId.Version is not a string", handler);
    std::string version = contentId["Version"];

    std::unique_ptr<ContentId> tmp;
    THROW_IF_FAILED_LOG(ContentId::Make(std::move(nameSpace), std::move(name), std::move(version), tmp), handler);

    return tmp;
}

std::unique_ptr<ContentId> GetLatestVersionResponseToContentId(const json& data, const ReportingHandler& handler)
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
    // We only query for one product at a time, so we expect only one result

    THROW_INVALID_RESPONSE_IF_FALSE(data.is_array(), "Response is not a JSON array", handler);
    THROW_INVALID_RESPONSE_IF_FALSE(data.size() == 1, "Response does not have the expected size", handler);

    const json& firstObj = data[0];
    THROW_INVALID_RESPONSE_IF_FALSE(firstObj.is_object(), "Response is not a JSON object", handler);
    THROW_INVALID_RESPONSE_IF_FALSE(firstObj.contains("ContentId"), "Missing ContentId in response", handler);

    return ContentIdJsonToObj(firstObj["ContentId"], handler);
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

    THROW_INVALID_RESPONSE_IF_FALSE(data.is_object(), "Response is not a JSON object", handler);

    return ContentIdJsonToObj(data["ContentId"], handler);
}

HashType HashTypeFromString(const std::string& hashType, const ReportingHandler& handler)
{
    if (AreEqualI(hashType, "Sha1"))
    {
        return HashType::Sha1;
    }
    else if (AreEqualI(hashType, "Sha256"))
    {
        return HashType::Sha256;
    }
    else
    {
        THROW_LOG(Result(Result::Unexpected, "Unknown hash type: " + hashType), handler);
    }
}

std::unique_ptr<File> FileJsonToObj(const json& file, const ReportingHandler& handler)
{
    THROW_INVALID_RESPONSE_IF_FALSE(file.is_object(), "File is not a JSON object", handler);

    THROW_INVALID_RESPONSE_IF_FALSE(file.contains("FileId"), "Missing File.FileId in response", handler);
    THROW_INVALID_RESPONSE_IF_FALSE(file["FileId"].is_string(), "File.FileId is not a string", handler);
    std::string fileId = file["FileId"];

    THROW_INVALID_RESPONSE_IF_FALSE(file.contains("Url"), "Missing File.Url in response", handler);
    THROW_INVALID_RESPONSE_IF_FALSE(file["Url"].is_string(), "File.Url is not a string", handler);
    std::string url = file["Url"];

    THROW_INVALID_RESPONSE_IF_FALSE(file.contains("SizeInBytes"), "Missing File.SizeInBytes in response", handler);
    THROW_INVALID_RESPONSE_IF_FALSE(file["SizeInBytes"].is_number_unsigned(),
                                    "File.SizeInBytes is not an unsigned number",
                                    handler);
    uint64_t sizeInBytes = file["SizeInBytes"];

    THROW_INVALID_RESPONSE_IF_FALSE(file.contains("Hashes"), "Missing File.Hashes in response", handler);
    THROW_INVALID_RESPONSE_IF_FALSE(file["Hashes"].is_object(), "File.Hashes is not an object", handler);
    std::unordered_map<HashType, std::string> hashes;
    for (const auto& [hashType, hashValue] : file["Hashes"].items())
    {
        THROW_INVALID_RESPONSE_IF_FALSE(hashValue.is_string(), "File.Hashes object value is not a string", handler);
        hashes[HashTypeFromString(hashType, handler)] = hashValue;
    }

    std::unique_ptr<File> tmp;
    THROW_IF_FAILED_LOG(File::Make(std::move(fileId), std::move(url), sizeInBytes, std::move(hashes), tmp), handler);

    return tmp;
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

    THROW_INVALID_RESPONSE_IF_FALSE(data.is_array(), "Response is not a JSON array", handler);

    // TODO #48: For now ignore DeliveryOptimization data. Will implement its separate parsing later

    std::vector<File> tmp;
    for (const auto& fileData : data)
    {
        THROW_INVALID_RESPONSE_IF_FALSE(fileData.is_object(), "Array element is not a JSON object", handler);
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
std::unique_ptr<ContentId> SFSClientImpl<ConnectionManagerT>::GetLatestVersion(const std::string& productName,
                                                                               const SearchAttributes& attributes,
                                                                               Connection& connection) const
try
{
    const std::string url{SFSUrlComponents::GetLatestVersionUrl(GetBaseUrl(), m_instanceId, m_nameSpace)};

    SFS_INFO("Requesting latest version of [%s] from URL [%s]", productName.c_str(), url.c_str());

    json targettingAttributes = json::object();
    for (const auto& [key, value] : attributes)
    {
        targettingAttributes[key] = value;
    }
    json body = json::array();
    body.push_back({{"TargetingAttributes", targettingAttributes}, {"Product", productName}});

    SFS_INFO("Request body [%s]", body.dump().c_str());

    const std::string postResponse{connection.Post(url, body.dump())};

    const json versionResponse = ParseServerMethodStringToJson(postResponse, "GetLatestVersion", m_reportingHandler);

    auto contentId = GetLatestVersionResponseToContentId(versionResponse, m_reportingHandler);
    THROW_CODE_IF_LOG(ServiceInvalidResponse,
                      !DoesGetVersionResponseMatchProduct(*contentId, m_nameSpace, productName),
                      m_reportingHandler,
                      "(GetLatestVersion) Response does not match the requested product");

    SFS_INFO("Received a response with version %s", contentId->GetVersion().c_str());

    return contentId;
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
