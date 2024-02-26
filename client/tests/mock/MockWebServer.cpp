// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "MockWebServer.h"

#include "../util/TestHelper.h"
#include "ErrorHandling.h"
#include "Util.h"
#include "connection/HttpHeader.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>

using namespace SFS;
using namespace SFS::details;
using namespace SFS::details::util;
using namespace SFS::test;
using namespace SFS::test::details;
using namespace std::string_literals;
using json = nlohmann::json;

#define BUILD_BUFFERED_LOG_DATA(message)                                                                               \
    BufferedLogData                                                                                                    \
    {                                                                                                                  \
        message, __FILE__, __LINE__, __FUNCTION__, std::chrono::system_clock::now()                                    \
    }

#define BUFFER_LOG(message) BufferLog(BUILD_BUFFERED_LOG_DATA(message))

const char* c_listenHostName = "localhost";

namespace
{
// TODO: Check if possible to update to new httplib enums after v0.14.3 https://github.com/microsoft/vcpkg/pull/36264
enum class StatusCode
{
    Ok = 200,
    BadRequest = 400,
    NotFound = 404,
    MethodNotAllowed = 405,
    InternalServerError = 500,
    ServiceUnavailable = 503,
};

std::string ToString(StatusCode status)
{
    switch (status)
    {
    case StatusCode::Ok:
        return "200 OK";
    case StatusCode::BadRequest:
        return "400 Bad Request";
    case StatusCode::NotFound:
        return "404 Not Found";
    case StatusCode::MethodNotAllowed:
        return "405 Method Not Allowed";
    case StatusCode::ServiceUnavailable:
        return "503 Service Unavailable";
    case StatusCode::InternalServerError:
        return "500 Internal Server Error";
    }

    return "";
}

class StatusCodeException : public std::exception
{
  public:
    StatusCodeException(StatusCode status) : m_status(status)
    {
    }

    const char* what() const noexcept override
    {
        return ToString(m_status).c_str();
    }

    int GetStatusCode() const
    {
        return static_cast<int>(m_status);
    }

  private:
    StatusCode m_status;
};

json GenerateContentIdJsonObject(const std::string& name, const std::string& latestVersion, const std::string& ns)
{
    // {
    //   "ContentId": {
    //     "Namespace": <ns>,
    //     "Name": <name>,
    //     "Version": <version>
    //   }
    // }

    return {{"ContentId", {{"Namespace", ns}, {"Name", name}, {"Version", latestVersion}}}};
}

json GeneratePostDownloadInfo(const std::string& name)
{
    // [
    //   {
    //     "Url": <url>,
    //     "FileId": <fileid>,
    //     "SizeInBytes": <size>,
    //     "Hashes": {
    //       "Sha1": <sha1>,
    //       "Sha256": <sha2>
    //     },
    //     "DeliveryOptimization": {
    //       "CatalogId": <catalogid>,
    //       "Properties": {
    //         "IntegrityCheckInfo": {
    //           "PiecesHashFileUrl": <url>,
    //           "HashOfHashes": <hash>
    //         }
    //       }
    //     }
    //   },
    //   ...
    // ]

    json response;
    response = json::array();
    response.push_back({{"Url", "http://localhost/1.json"},
                        {"FileId", name + ".json"},
                        {"SizeInBytes", 100},
                        {"Hashes", {{"Sha1", "123"}, {"Sha256", "456"}}}});
    response[0]["DeliveryOptimization"] = {{"CatalogId", "789"}};
    response[0]["DeliveryOptimization"]["Properties"] = {
        {"IntegrityCheckInfo", {{"PiecesHashFileUrl", "http://localhost/1.json"}, {"HashOfHashes", "abc"}}}};

    response.push_back({{"Url", "http://localhost/2.bin"},
                        {"FileId", name + ".bin"},
                        {"SizeInBytes", 200},
                        {"Hashes", {{"Sha1", "421"}, {"Sha256", "132"}}}});
    response[1]["DeliveryOptimization"] = {{"CatalogId", "14"}};
    response[1]["DeliveryOptimization"]["Properties"] = {
        {"IntegrityCheckInfo", {{"PiecesHashFileUrl", "http://localhost/2.bin"}, {"HashOfHashes", "abcd"}}}};
    return response;
}

struct BufferedLogData
{
    std::string message;
    std::string file;
    unsigned line;
    std::string function;
    std::chrono::time_point<std::chrono::system_clock> time;
};

LogData ToLogData(const BufferedLogData& data)
{
    return {LogSeverity::Info, data.message.c_str(), data.file.c_str(), data.line, data.function.c_str(), data.time};
}

void CheckApiVersion(const httplib::Request& req, std::string_view apiVersion)
{
    if (util::AreNotEqualI(req.path_params.at("apiVersion"), apiVersion))
    {
        throw StatusCodeException(StatusCode::NotFound);
    }
}
} // namespace

namespace SFS::test::details
{
class MockWebServerImpl
{
  public:
    MockWebServerImpl() = default;
    ~MockWebServerImpl() = default;

    MockWebServerImpl(const MockWebServerImpl&) = delete;
    MockWebServerImpl& operator=(const MockWebServerImpl&) = delete;

    void Start();
    Result Stop();

    std::string GetUrl() const;

    void RegisterProduct(std::string&& name, std::string&& version);
    void RegisterExpectedRequestHeader(std::string&& header, std::string&& value);

  private:
    void ConfigureServerSettings();
    void ConfigureRequestHandlers();

    void ConfigurePostLatestVersion();
    void ConfigurePostLatestVersionBatch();
    void ConfigureGetSpecificVersion();
    void ConfigurePostDownloadInfo();

    void RunHttpCallback(const httplib::Request& req,
                         httplib::Response& res,
                         const std::string& methodName,
                         const std::string& apiVersion,
                         const std::function<void(const httplib::Request, httplib::Response&)>& callback);
    void CheckRequestHeaders(const httplib::Request& req);

    void BufferLog(const BufferedLogData& data);
    void ProcessBufferedLogs();

    httplib::Server m_server;
    int m_port{-1};

    std::optional<Result> m_lastException;

    std::thread m_listenerThread;

    using VersionList = std::set<std::string>;
    std::unordered_map<std::string, VersionList> m_products;

    std::unordered_map<std::string, std::string> m_expectedRequestHeaders;

    std::vector<BufferedLogData> m_bufferedLog;
    std::mutex m_logMutex;
};
} // namespace SFS::test::details

MockWebServer::MockWebServer()
{
    m_impl = std::make_unique<MockWebServerImpl>();
    m_impl->Start();
}

MockWebServer::~MockWebServer()
{
    const auto ret = Stop();
    if (!ret)
    {
        TEST_UNSCOPED_INFO("Failed to stop: " + std::string(ToString(ret.GetCode())));
    }
}

Result MockWebServer::Stop()
{
    return m_impl->Stop();
}

std::string MockWebServer::GetBaseUrl() const
{
    return m_impl->GetUrl();
}

void MockWebServer::RegisterProduct(std::string name, std::string version)
{
    m_impl->RegisterProduct(std::move(name), std::move(version));
}

void MockWebServer::RegisterExpectedRequestHeader(HttpHeader header, std::string value)
{
    std::string headerName = ToString(header);
    m_impl->RegisterExpectedRequestHeader(std::move(headerName), std::move(value));
}

void MockWebServerImpl::Start()
{
    ConfigureServerSettings();
    ConfigureRequestHandlers();

    m_port = m_server.bind_to_any_port(c_listenHostName);
    m_listenerThread = std::thread([&]() { m_server.listen_after_bind(); });
}

void MockWebServerImpl::ConfigureServerSettings()
{
    m_server.set_logger([&](const httplib::Request& req, const httplib::Response& res) {
        BUFFER_LOG("Request: " + req.method + " " + req.path + " " + req.version);
        BUFFER_LOG("Request Body: " + req.body);

        BUFFER_LOG("Response: " + res.version + " " + ToString(static_cast<StatusCode>(res.status)) + " " + res.reason);
        BUFFER_LOG("Response body: " + res.body);
    });

    m_server.set_exception_handler([&](const httplib::Request&, httplib::Response& res, std::exception_ptr ep) {
        try
        {
            std::rethrow_exception(ep);
        }
        catch (std::exception& e)
        {
            m_lastException = Result(Result::HttpUnexpected, e.what());
        }
        catch (...)
        {
            m_lastException = Result(Result::HttpUnexpected, "Unknown Exception");
        }

        ProcessBufferedLogs();

        res.status = static_cast<int>(StatusCode::InternalServerError);
    });

    // Keeping this interval to a minimum ensures tests run quicker
    m_server.set_keep_alive_timeout(1); // 1 second
}

void MockWebServerImpl::ConfigureRequestHandlers()
{
    ConfigurePostLatestVersion();
    ConfigurePostLatestVersionBatch();
    ConfigureGetSpecificVersion();
    ConfigurePostDownloadInfo();
}

void MockWebServerImpl::ConfigurePostLatestVersion()
{
    // Path: /api/<apiVersion:v2>/contents/<instanceId>/namespaces/<ns>/names/<name>/versions/latest?action=select
    const std::string pattern = "/api/:apiVersion/contents/:instanceId/namespaces/:ns/names/:name/versions/latest";
    m_server.Post(pattern, [&](const httplib::Request& req, httplib::Response& res) {
        RunHttpCallback(req, res, "PostLatestVersion", "v2", [&](const httplib::Request& req, httplib::Response& res) {
            // TODO: Ignoring instanceId for now

            if (!req.has_param("action") || util::AreNotEqualI(req.get_param_value("action"), "select"))
            {
                // TODO: SFS might throw a different error when the query string is unexpected
                throw StatusCodeException(StatusCode::NotFound);
            }

            // Checking body has expected format, but won't use it for the response
            {
                if (req.body.empty())
                {
                    throw StatusCodeException(StatusCode::BadRequest);
                }

                json body;
                try
                {
                    body = json::parse(req.body);
                }
                catch (const json::parse_error& ex)
                {
                    BUFFER_LOG("JSON parse error: " + std::string(ex.what()));
                    throw StatusCodeException(StatusCode::BadRequest);
                }

                // The GetLatestVersion API expects an object as a body, with a "TargetingAttributes" object element.
                if (!body.is_object() || !body.contains("TargetingAttributes") ||
                    !body["TargetingAttributes"].is_object())
                {
                    throw StatusCodeException(StatusCode::BadRequest);
                }
            }

            const std::string& name = req.path_params.at("name");
            auto it = m_products.find(name);
            if (it == m_products.end())
            {
                throw StatusCodeException(StatusCode::NotFound);
            }

            const VersionList& versions = it->second;
            if (versions.empty())
            {
                throw StatusCodeException(StatusCode::InternalServerError);
            }

            const std::string ns = req.path_params.at("ns");
            const auto& latestVersion = *versions.rbegin();

            const json response = GenerateContentIdJsonObject(name, latestVersion, ns);

            res.set_content(response.dump(), "application/json");
        });
    });
}

void MockWebServerImpl::ConfigurePostLatestVersionBatch()
{
    // Path: /api/<apiVersion:v2>/contents/<instanceId>/namespaces/<ns>/names?action=BatchUpdates
    const std::string pattern = "/api/:apiVersion/contents/:instanceId/namespaces/:ns/names";
    m_server.Post(pattern, [&](const httplib::Request& req, httplib::Response& res) {
        RunHttpCallback(
            req,
            res,
            "PostLatestVersionBatch",
            "v2",
            [&](const httplib::Request& req, httplib::Response& res) {
                // TODO: Ignoring instanceId for now

                if (!req.has_param("action") || util::AreNotEqualI(req.get_param_value("action"), "BatchUpdates"))
                {
                    // TODO: SFS might throw a different error when the query string is unexpected
                    throw StatusCodeException(StatusCode::NotFound);
                }

                if (req.body.empty())
                {
                    throw StatusCodeException(StatusCode::BadRequest);
                }

                json body;
                try
                {
                    body = json::parse(req.body);
                }
                catch (const json::parse_error& ex)
                {
                    BUFFER_LOG("JSON parse error: " + std::string(ex.what()));
                    throw StatusCodeException(StatusCode::BadRequest);
                }

                // The BatchUpdates API returns an array of objects, each with a "Product" key.
                // If repeated, the same product is only returned once.
                // TODO: We are ignoring the TargetingAttributes for now.
                if (!body.is_array())
                {
                    throw StatusCodeException(StatusCode::BadRequest);
                }

                // Iterate over the array and collect the unique products
                std::unordered_map<std::string, json> requestedProducts;
                for (const auto& productRequest : body)
                {
                    if (!productRequest.is_object() || !productRequest.contains("Product") ||
                        !productRequest["Product"].is_string() || !productRequest.contains("TargetingAttributes"))
                    {
                        throw StatusCodeException(StatusCode::BadRequest);
                    }
                    if (requestedProducts.count(productRequest["Product"]))
                    {
                        continue;
                    }
                    requestedProducts.emplace(productRequest["Product"], productRequest["TargetingAttributes"]);
                }

                // If at least one product exists, we will return a 200 OK with that. Non-existing products are ignored.
                // Otherwise, a 404 is sent.
                json response = json::array();
                for (const auto& [name, _] : requestedProducts)
                {
                    auto it = m_products.find(name);
                    if (it == m_products.end())
                    {
                        continue;
                    }

                    const VersionList& versions = it->second;
                    if (versions.empty())
                    {
                        res.status = static_cast<int>(StatusCode::InternalServerError);
                        return;
                    }

                    const std::string ns = req.path_params.at("ns");
                    const auto& latestVersion = *versions.rbegin();

                    response.push_back(GenerateContentIdJsonObject(name, latestVersion, ns));
                }

                if (response.empty())
                {
                    throw StatusCodeException(StatusCode::NotFound);
                }

                res.set_content(response.dump(), "application/json");
            });
    });
}

void MockWebServerImpl::ConfigureGetSpecificVersion()
{
    // Path: /api/<apiVersion:v2>/contents/<instanceId>/namespaces/<ns>/names/<name>/versions/<version>
    const std::string pattern = "/api/:apiVersion/contents/:instanceId/namespaces/:ns/names/:name/versions/:version";
    m_server.Get(pattern, [&](const httplib::Request& req, httplib::Response& res) {
        RunHttpCallback(req, res, "GetSpecificVersion", "v2", [&](const httplib::Request& req, httplib::Response& res) {
            // TODO: Ignoring instanceId for now

            const std::string& name = req.path_params.at("name");
            auto it = m_products.find(name);
            if (it == m_products.end())
            {
                throw StatusCodeException(StatusCode::NotFound);
            }

            const VersionList& versions = it->second;
            if (versions.empty())
            {
                throw StatusCodeException(StatusCode::InternalServerError);
            }

            const std::string& version = req.path_params.at("version");
            if (version.empty() || !versions.count(version))
            {
                throw StatusCodeException(StatusCode::NotFound);
            }

            const std::string ns = req.path_params.at("ns");

            res.set_content(GenerateContentIdJsonObject(name, version, ns).dump(), "application/json");
        });
    });
}

void MockWebServerImpl::ConfigurePostDownloadInfo()
{
    // Path:
    // /api/<apiVersion:v2>/contents/<instanceId>/namespaces/<ns>/names/<name>/versions/<version>/files?action=GenerateDownloadInfo
    const std::string pattern =
        "/api/:apiVersion/contents/:instanceId/namespaces/:ns/names/:name/versions/:version/files";
    m_server.Post(pattern, [&](const httplib::Request& req, httplib::Response& res) {
        RunHttpCallback(req, res, "PostDownloadInfo", "v2", [&](const httplib::Request& req, httplib::Response& res) {
            // TODO: Ignoring instanceId and ns for now

            if (!req.has_param("action") || util::AreNotEqualI(req.get_param_value("action"), "GenerateDownloadInfo"))
            {
                // TODO: SFS might throw a different error when the query string is unexpected
                throw StatusCodeException(StatusCode::NotFound);
            }

            const std::string& name = req.path_params.at("name");
            auto it = m_products.find(name);
            if (it == m_products.end())
            {
                throw StatusCodeException(StatusCode::NotFound);
            }

            const VersionList& versions = it->second;
            if (versions.empty())
            {
                throw StatusCodeException(StatusCode::InternalServerError);
            }

            const std::string& version = req.path_params.at("version");
            if (version.empty() || !versions.count(version))
            {
                throw StatusCodeException(StatusCode::NotFound);
            }

            // Response is a dummy, doesn't use the version above
            res.set_content(GeneratePostDownloadInfo(name).dump(), "application/json");
        });
    });
}

void MockWebServerImpl::RunHttpCallback(const httplib::Request& req,
                                        httplib::Response& res,
                                        const std::string& methodName,
                                        const std::string& apiVersion,
                                        const std::function<void(const httplib::Request, httplib::Response&)>& callback)
{
    try
    {
        BUFFER_LOG("Matched " + methodName);
        CheckApiVersion(req, apiVersion);
        CheckRequestHeaders(req);
        callback(req, res);
        res.status = static_cast<int>(StatusCode::Ok);
    }
    catch (const StatusCodeException& ex)
    {
        res.status = ex.GetStatusCode();
    }
    catch (const std::exception&)
    {
        res.status = static_cast<int>(StatusCode::InternalServerError);
    }
    catch (...)
    {
        res.status = static_cast<int>(StatusCode::InternalServerError);
    }
}

void MockWebServerImpl::CheckRequestHeaders(const httplib::Request& req)
{
    for (const auto& header : m_expectedRequestHeaders)
    {
        std::optional<std::string> errorMessage;
        if (!req.has_header(header.first))
        {
            errorMessage = "Expected header [" + header.first + "] not found";
        }
        else if (util::AreNotEqualI(req.get_header_value(header.first), header.second))
        {
            errorMessage = "Header [" + header.first + "] with value [" + req.get_header_value(header.first) +
                           "] does not match the expected value [" + header.second + "]";
        }

        if (errorMessage)
        {
            BUFFER_LOG(*errorMessage);
            throw std::runtime_error(errorMessage->c_str());
        }
    }
}

void MockWebServerImpl::BufferLog(const BufferedLogData& data)
{
    std::lock_guard guard(m_logMutex);
    m_bufferedLog.push_back(data);
}

void MockWebServerImpl::ProcessBufferedLogs()
{
    for (const auto& data : m_bufferedLog)
    {
        LogCallbackToTest(ToLogData(data));
    }
    m_bufferedLog.clear();
}

Result MockWebServerImpl::Stop()
{
    if (m_listenerThread.joinable())
    {
        m_server.stop();
        m_listenerThread.join();
    }
    ProcessBufferedLogs();
    return m_lastException.value_or(Result::Success);
}

std::string MockWebServerImpl::GetUrl() const
{
    return "http://"s + c_listenHostName + ":"s + std::to_string(m_port);
}

void MockWebServerImpl::RegisterProduct(std::string&& name, std::string&& version)
{
    m_products[std::move(name)].emplace(std::move(version));
}

void MockWebServerImpl::RegisterExpectedRequestHeader(std::string&& header, std::string&& value)
{
    m_expectedRequestHeaders.emplace(std::move(header), std::move(value));
}
