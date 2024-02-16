// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "MockWebServer.h"

#include "../util/TestHelper.h"
#include "ErrorHandling.h"
#include "Util.h"

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

json GenerateGetSpecificVersionResponse(const std::string& name, const std::string& version, const std::string& ns)
{
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

    json response;
    response["ContentId"] = {{"Namespace", ns}, {"Name", name}, {"Version", version}};
    response["Files"] = json::array({name + ".json", name + ".bin"});
    return response;
}

json GeneratePostLatestVersionResponse(const std::string& name, const std::string& latestVersion, const std::string& ns)
{
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

    json response;
    response = json::array();
    response.push_back({{"ContentId", {{"Namespace", ns}, {"Name", name}, {"Version", latestVersion}}}});
    return response;
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
    void RegisterExpectedHeader(std::string&& header, std::string&& value);

  private:
    void ConfigureServerSettings();
    void ConfigureRequestHandlers();

    void ConfigurePostLatestVersion();
    void ConfigureGetSpecificVersion();
    void ConfigurePostDownloadInfo();

    void CheckHeaders(const httplib::Request& req);

    void BufferLog(const BufferedLogData& data);
    void ProcessBufferedLogs();

    httplib::Server m_server;
    int m_port{-1};

    std::optional<Result> m_lastException;

    std::thread m_listenerThread;

    using VersionList = std::set<std::string>;
    std::unordered_map<std::string, VersionList> m_products;

    std::unordered_map<std::string, std::string> m_expectedHeaders;

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

void MockWebServer::RegisterExpectedHeader(std::string header, std::string value)
{
    m_impl->RegisterExpectedHeader(std::move(header), std::move(value));
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
    ConfigureGetSpecificVersion();
    ConfigurePostDownloadInfo();
}

void MockWebServerImpl::ConfigurePostLatestVersion()
{
    // Path: /api/<apiVersion:v2>/contents/<instanceId>/namespaces/<ns>/names?action=BatchUpdates
    const std::string pattern = "/api/:apiVersion/contents/:instanceId/namespaces/:ns/names";
    m_server.Post(pattern, [&](const httplib::Request& req, httplib::Response& res) {
        BUFFER_LOG("Matched PostLatestVersion");
        CheckHeaders(req);

        if (util::AreNotEqualI(req.path_params.at("apiVersion"), "v2"))
        {
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        // TODO: Ignoring instanceId for now

        if (!req.has_param("action") || util::AreNotEqualI(req.get_param_value("action"), "BatchUpdates"))
        {
            // TODO: SFS might throw a different error when the query string is unexpected
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        if (req.body.empty())
        {
            res.status = static_cast<int>(StatusCode::BadRequest);
            return;
        }

        json body;
        try
        {
            body = json::parse(req.body);
        }
        catch (const json::parse_error& ex)
        {
            BUFFER_LOG("JSON parse error: " + std::string(ex.what()));
            res.status = static_cast<int>(StatusCode::BadRequest);
            return;
        }

        // The BatchUpdates API returns an array of objects, each with a "Product" key.
        // For this mock we're only interested in the first one.
        // In the future we may want to support multiple products in a single request.
        std::string name;
        if (body.is_array() && body[0].is_object() && body[0].contains("Product"))
        {
            name = body[0]["Product"];
        }
        else
        {
            res.status = static_cast<int>(StatusCode::BadRequest);
            return;
        }

        auto it = m_products.find(name);
        if (it == m_products.end())
        {
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        const VersionList& versions = it->second;
        if (versions.empty())
        {
            res.status = static_cast<int>(StatusCode::InternalServerError);
            return;
        }

        const std::string ns = req.path_params.at("ns");
        const auto& latestVersion = *versions.rbegin();

        res.status = static_cast<int>(StatusCode::Ok);
        res.set_content(GeneratePostLatestVersionResponse(name, latestVersion, ns).dump(), "application/json");
    });
}

void MockWebServerImpl::ConfigureGetSpecificVersion()
{
    // Path: /api/<apiVersion:v1>/contents/<instanceId>/namespaces/<ns>/names/<name>/versions/<version>
    const std::string pattern = "/api/:apiVersion/contents/:instanceId/namespaces/:ns/names/:name/versions/:version";
    m_server.Get(pattern, [&](const httplib::Request& req, httplib::Response& res) {
        BUFFER_LOG("Matched GetSpecificVersion");
        CheckHeaders(req);

        if (util::AreNotEqualI(req.path_params.at("apiVersion"), "v1"))
        {
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        // TODO: Ignoring instanceId for now

        const std::string& name = req.path_params.at("name");
        auto it = m_products.find(name);
        if (it == m_products.end())
        {
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        const VersionList& versions = it->second;
        if (versions.empty())
        {
            res.status = static_cast<int>(StatusCode::InternalServerError);
            return;
        }

        const std::string& version = req.path_params.at("version");
        if (version.empty() || !versions.count(version))
        {
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        const std::string ns = req.path_params.at("ns");

        res.status = static_cast<int>(StatusCode::Ok);
        res.set_content(GenerateGetSpecificVersionResponse(name, version, ns).dump(), "application/json");
    });
}

void MockWebServerImpl::ConfigurePostDownloadInfo()
{
    // Path:
    // /api/<apiVersion:v1>/contents/<instanceId>/namespaces/<ns>/names/<name>/versions/<version>/files?action=GenerateDownloadInfo
    const std::string pattern =
        "/api/:apiVersion/contents/:instanceId/namespaces/:ns/names/:name/versions/:version/files";
    m_server.Post(pattern, [&](const httplib::Request& req, httplib::Response& res) {
        BUFFER_LOG("Matched PostDownloadInfo");
        CheckHeaders(req);

        if (util::AreNotEqualI(req.path_params.at("apiVersion"), "v1"))
        {
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        // TODO: Ignoring instanceId and ns for now

        if (!req.has_param("action") || util::AreNotEqualI(req.get_param_value("action"), "GenerateDownloadInfo"))
        {
            // TODO: SFS might throw a different error when the query string is unexpected
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        const std::string& name = req.path_params.at("name");
        auto it = m_products.find(name);
        if (it == m_products.end())
        {
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        const VersionList& versions = it->second;
        if (versions.empty())
        {
            res.status = static_cast<int>(StatusCode::InternalServerError);
            return;
        }

        const std::string& version = req.path_params.at("version");
        if (version.empty() || !versions.count(version))
        {
            res.status = static_cast<int>(StatusCode::NotFound);
            return;
        }

        // Response is a dummy, doesn't use the version above

        res.status = static_cast<int>(StatusCode::Ok);
        res.set_content(GeneratePostDownloadInfo(name).dump(), "application/json");
    });
}

void MockWebServerImpl::CheckHeaders(const httplib::Request& req)
{
    for (const auto& header : m_expectedHeaders)
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
            throw std::exception(errorMessage->c_str());
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

void MockWebServerImpl::RegisterExpectedHeader(std::string&& header, std::string&& value)
{
    m_expectedHeaders.emplace(std::move(header), std::move(value));
}
