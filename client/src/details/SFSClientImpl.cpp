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
#define SFS_RETURN_IF_FAILED(result) RETURN_IF_FAILED_LOG(result, m_reportingHandler)
#define CHECK_JSON(condition, message) RETURN_CODE_IF_LOG(ServiceInvalidResponse, condition, handler, message)

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

Result ParseStringToJson(const std::string& data, const std::string& method, json& out)
{
    try
    {
        out = json::parse(data);
    }
    catch (json::parse_error& ex)
    {
        return Result(Result::ServiceInvalidResponse, "(" + method + ") JSON Parsing error: " + std::string(ex.what()));
    }
    return Result::Success;
}

Result ContentIdJsonToObj(const nlohmann::json& contentId,
                          const ReportingHandler& handler,
                          std::unique_ptr<ContentId>& out)
{
    CHECK_JSON(!contentId.is_object(), "ContentId is not a JSON object");

    CHECK_JSON(!contentId.contains("Namespace"), "Missing ContentId.Namespace in response");
    CHECK_JSON(!contentId["Namespace"].is_string(), "ContentId.Namespace is not a string");
    std::string nameSpace = contentId["Namespace"].get<std::string>();

    CHECK_JSON(!contentId.contains("Name"), "Missing ContentId.Name in response");
    CHECK_JSON(!contentId["Name"].is_string(), "ContentId.Name is not a string");
    std::string name = contentId["Name"].get<std::string>();

    CHECK_JSON(!contentId.contains("Version"), "Missing ContentId.Version in response");
    CHECK_JSON(!contentId["Version"].is_string(), "ContentId.Version is not a string");
    std::string version = contentId["Version"].get<std::string>();

    return ContentId::Make(std::move(nameSpace), std::move(name), std::move(version), out);
}

Result GetLatestVersionResponseToContentId(const nlohmann::json& data,
                                           const ReportingHandler& handler,
                                           std::unique_ptr<ContentId>& out)
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

    CHECK_JSON(!data.is_array(), "Response is not a JSON array");
    CHECK_JSON(data.size() != 1, "Response does not have the expected size");

    const json& firstObj = data[0];
    CHECK_JSON(!firstObj.is_object(), "Response is not a JSON object");
    CHECK_JSON(!firstObj.contains("ContentId"), "Missing ContentId in response");

    return ContentIdJsonToObj(firstObj["ContentId"], handler, out);
}

Result GetSpecificVersionResponseToContentId(const nlohmann::json& data,
                                             const ReportingHandler& handler,
                                             std::unique_ptr<ContentId>& out)
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

    CHECK_JSON(!data.is_object(), "Response is not a JSON object");

    return ContentIdJsonToObj(data["ContentId"], handler, out);
}

Result HashTypeFromString(const std::string& hashType, HashType& out)
{
    if (AreEqualI(hashType, "Sha1"))
    {
        out = HashType::Sha1;
    }
    else if (AreEqualI(hashType, "Sha256"))
    {
        out = HashType::Sha256;
    }
    else
    {
        return Result(Result::Unexpected, "Unknown hash type: " + hashType);
    }
    return Result::Success;
}

Result FileJsonToObj(const nlohmann::json& file, const ReportingHandler& handler, std::unique_ptr<File>& out)
{
    CHECK_JSON(!file.is_object(), "File is not a JSON object");

    CHECK_JSON(!file.contains("FileId"), "Missing File.FileId in response");
    CHECK_JSON(!file["FileId"].is_string(), "File.FileId is not a string");
    std::string fileId = file["FileId"].get<std::string>();

    CHECK_JSON(!file.contains("Url"), "Missing File.Url in response");
    CHECK_JSON(!file["Url"].is_string(), "File.Url is not a string");
    std::string url = file["Url"].get<std::string>();

    CHECK_JSON(!file.contains("SizeInBytes"), "Missing File.SizeInBytes in response");
    CHECK_JSON(!file["SizeInBytes"].is_number_unsigned(), "File.SizeInBytes is not an unsigned number");
    uint64_t sizeInBytes = file["SizeInBytes"].get<uint64_t>();

    CHECK_JSON(!file.contains("Hashes"), "Missing File.Hashes in response");
    CHECK_JSON(!file["Hashes"].is_object(), "File.Hashes is not an object");
    std::unordered_map<HashType, std::string> hashes;
    for (const auto& [hashType, hashValue] : file["Hashes"].items())
    {
        CHECK_JSON(!hashValue.is_string(), "File.Hashes object value is not a string");

        HashType type;
        RETURN_IF_FAILED_LOG(HashTypeFromString(hashType, type), handler);
        hashes[type] = hashValue.get<std::string>();
    }

    return File::Make(std::move(fileId), std::move(url), sizeInBytes, std::move(hashes), out);
}

Result GetDownloadInfoResponseToFileVector(const nlohmann::json& data,
                                           const ReportingHandler& handler,
                                           std::vector<std::unique_ptr<File>>& out)
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

    CHECK_JSON(!data.is_array(), "Response is not a JSON array");

    // TODO #48: For now ignore DeliveryOptimization data. Will implement its separate parsing later

    std::vector<std::unique_ptr<File>> tmp;
    for (const auto& fileData : data)
    {
        CHECK_JSON(!fileData.is_object(), "Array element is not a JSON object");

        std::unique_ptr<File> file;
        RETURN_IF_FAILED_LOG(FileJsonToObj(fileData, handler, file), handler);
        tmp.push_back(std::move(file));
    }

    out = std::move(tmp);

    return Result::Success;
}

bool IsVersionResponseValid(const ContentId& contentId, std::string_view nameSpace, std::string_view name)
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
Result SFSClientImpl<ConnectionManagerT>::GetLatestVersion(const std::string& productName,
                                                           const SearchAttributes& attributes,
                                                           Connection& connection,
                                                           std::unique_ptr<ContentId>& contentId) const
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

    std::string out;
    SFS_RETURN_IF_FAILED(connection.Post(url, body.dump(), out));

    json response;
    SFS_RETURN_IF_FAILED(ParseStringToJson(out, "GetLatestVersion", response));

    std::unique_ptr<ContentId> tmp;
    SFS_RETURN_IF_FAILED(GetLatestVersionResponseToContentId(response, m_reportingHandler, tmp));
    RETURN_CODE_IF_LOG(ServiceInvalidResponse,
                       !IsVersionResponseValid(*tmp, m_nameSpace, productName),
                       m_reportingHandler,
                       "(GetLatestVersion) Response does not match the requested product");

    contentId = std::move(tmp);

    return Result::Success;
}

template <typename ConnectionManagerT>
Result SFSClientImpl<ConnectionManagerT>::GetSpecificVersion(const std::string& productName,
                                                             const std::string& version,
                                                             Connection& connection,
                                                             std::unique_ptr<ContentId>& contentId) const
{
    const std::string url{
        SFSUrlComponents::GetSpecificVersionUrl(GetBaseUrl(), m_instanceId, m_nameSpace, productName, version)};

    SFS_INFO("Requesting version [%s] of [%s] from URL [%s]", version.c_str(), productName.c_str(), url.c_str());

    std::string out;
    SFS_RETURN_IF_FAILED(connection.Get(url, out));

    json response;
    SFS_RETURN_IF_FAILED(ParseStringToJson(out, "GetSpecificVersion", response));

    std::unique_ptr<ContentId> tmp;
    SFS_RETURN_IF_FAILED(GetSpecificVersionResponseToContentId(response, m_reportingHandler, tmp));
    RETURN_CODE_IF_LOG(ServiceInvalidResponse,
                       !IsVersionResponseValid(*tmp, m_nameSpace, productName),
                       m_reportingHandler,
                       "(GetSpecificVersion) Response does not match the requested product");

    contentId = std::move(tmp);

    return Result::Success;
}

template <typename ConnectionManagerT>
Result SFSClientImpl<ConnectionManagerT>::GetDownloadInfo(const std::string& productName,
                                                          const std::string& version,
                                                          Connection& connection,
                                                          std::vector<std::unique_ptr<File>>& files) const
{
    const std::string url{
        SFSUrlComponents::GetDownloadInfoUrl(GetBaseUrl(), m_instanceId, m_nameSpace, productName, version)};

    SFS_INFO("Requesting download info of version [%s] of [%s] from URL [%s]",
             version.c_str(),
             productName.c_str(),
             url.c_str());

    std::string out;
    SFS_RETURN_IF_FAILED(connection.Post(url, out));

    json response;
    SFS_RETURN_IF_FAILED(ParseStringToJson(out, "GetDownloadInfo", response));

    std::vector<std::unique_ptr<File>> tmp;
    SFS_RETURN_IF_FAILED(GetDownloadInfoResponseToFileVector(response, m_reportingHandler, tmp));

    files = std::move(tmp);

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
