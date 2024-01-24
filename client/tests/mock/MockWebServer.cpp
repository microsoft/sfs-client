// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "MockWebServer.h"

#include "ErrorHandling.h"
#include "HttpRequestResponse.h"
#include "Util.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <thread>

#define MAX_LOG_MESSAGE_SIZE 1000

#ifdef _WIN32

using socket_length_t = int;

#else

// Sockets are int values in Linux
using SOCKET = int;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

// These defines have same behavior but different names
#define SD_BOTH SHUT_RDWR
#define SD_RECEIVE SHUT_RD
#define SD_SEND SHUT_WR

using socket_length_t = socklen_t;

#endif

using namespace SFS;
using namespace SFS::details;
using namespace SFS::details::util;
using namespace SFS::test;
using namespace SFS::test::details;
using json = nlohmann::json;

// Keep these two in sync.
const char* c_listenHostName = "localhost";
const char* c_listenHostIp4Addr = "127.0.0.1";

namespace
{
class ScopedSocket
{
  public:
    ScopedSocket();
    ~ScopedSocket();

    ScopedSocket(SOCKET socket);

    SOCKET Get() const
    {
        return m_socket;
    }

  private:
    SOCKET m_socket = INVALID_SOCKET;
};

enum class ApiMethod
{
    GetSpecificVersion,
    PostLatestVersion,
    PostDownloadInfo
};

void ThrowLastSocketError(const std::string& activity)
{
#ifdef _WIN32
    const std::string errorCode = std::to_string(WSAGetLastError());
#else
    const std::string errorCode = std::to_string(errno);
#endif
    throw SFSException(Result::E_Unexpected, activity + " failed with error: " + errorCode);
}

void ThrowSocketError(const std::string& activity)
{
    throw SFSException(Result::E_Unexpected, activity + " failed");
}

void GenerateGetSpecificVersionResponse(const std::string& name, const std::string& version, json& jsonResponse)
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

    jsonResponse["ContentId"] = {{"Namespace", "default"}, {"Name", name}, {"Version", version}};
    jsonResponse["Files"] = json::array({name + ".json", name + ".bin"});
}

void GeneratePostLatestVersionResponse(const std::string& name, const std::string& latestVersion, json& jsonResponse)
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

    jsonResponse = json::array();
    jsonResponse.push_back({{"ContentId", {{"Namespace", "default"}, {"Name", name}, {"Version", latestVersion}}}});
}

void GeneratePostDownloadInfo(const std::string& name, json& jsonResponse)
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

    jsonResponse = json::array();
    jsonResponse.push_back({{"Url", "http://localhost/1.json"},
                            {"FileId", name + ".json"},
                            {"SizeInBytes", 100},
                            {"Hashes", {{"Sha1", "123"}, {"Sha256", "456"}}}});
    jsonResponse[0]["DeliveryOptimization"] = {{"CatalogId", "789"}};
    jsonResponse[0]["DeliveryOptimization"]["Properties"] = {
        {"IntegrityCheckInfo", {{"PiecesHashFileUrl", "http://localhost/1.json"}, {"HashOfHashes", "abc"}}}};

    jsonResponse.push_back({{"Url", "http://localhost/2.bin"},
                            {"FileId", name + ".bin"},
                            {"SizeInBytes", 200},
                            {"Hashes", {{"Sha1", "421"}, {"Sha256", "132"}}}});
    jsonResponse[1]["DeliveryOptimization"] = {{"CatalogId", "14"}};
    jsonResponse[1]["DeliveryOptimization"]["Properties"] = {
        {"IntegrityCheckInfo", {{"PiecesHashFileUrl", "http://localhost/2.bin"}, {"HashOfHashes", "abcd"}}}};
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

    void RegisterProduct(const std::string& name, const std::string& version);

    static void StaticListen(MockWebServerImpl& server);

  private:
    void SetupSocket();
    void StartListening();

    void Listen();
    void ProcessRequest(ScopedSocket& clientSocket) const;
    void ReceiveRequest(ScopedSocket& clientSocket, std::unique_ptr<HttpRequest>& request) const;
    void GenerateResponse(const HttpRequest& request, HttpResponse& response) const;
    void SendResponse(ScopedSocket& clientSocket, const std::string& response) const;

    ApiMethod GetApiMethod(const HttpRequest& request, std::optional<std::string>& version, std::string& name) const;

    std::string GetBindPort() const;

    std::mutex m_socketMutex;
    std::unique_ptr<ScopedSocket> m_socket;

    std::thread m_listenerThread;
    std::optional<Result> m_listenerThreadResult;

    bool m_shutdown{false};
    std::mutex m_shutdownMutex;

    bool m_isRunning{false};

    using VersionList = std::set<std::string>;
    std::unordered_map<std::string, VersionList> m_products;
};
} // namespace SFS::test::details

MockWebServer::MockWebServer()
{
    m_impl = std::make_unique<MockWebServerImpl>();
    m_impl->Start();
}

MockWebServer::~MockWebServer()
{
    m_impl->Stop();
}

Result MockWebServer::Stop()
{
    return m_impl->Stop();
}

std::string MockWebServer::GetBaseUrl() const
{
    return m_impl->GetUrl();
}

void MockWebServer::RegisterProduct(const std::string& name, const std::string& version)
{
    m_impl->RegisterProduct(name, version);
}

void MockWebServerImpl::Start()
{
#ifdef _WIN32
    // Initialize the latest WinSock 2.2
    WSADATA wsaData;
    if (auto err = WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        throw SFSException(Result::E_Unexpected, "WSAStartup failed with error: " + std::to_string(err));
    }
#endif
    m_isRunning = true;

    SetupSocket();
    StartListening();
}

void MockWebServerImpl::SetupSocket()
{
    m_socket = std::make_unique<ScopedSocket>();

    sockaddr_in sin{};
    sin.sin_family = AF_INET;

    // Convert the address into a binary form
    const auto inetRet = inet_pton(AF_INET, c_listenHostIp4Addr, &sin.sin_addr.s_addr);
    if (inetRet == -1)
    {
        ThrowLastSocketError("Socket setup");
    }
    else if (inetRet == 0)
    {
        ThrowSocketError("Socket setup");
    }

    // Assign any available port from dynamic client range
    sin.sin_port = ::htons(0);

    // Bind the socket to the specified address
    if (bind(m_socket->Get(), (const struct sockaddr*)&sin, static_cast<int>(sizeof(sin))) == SOCKET_ERROR)
    {
        ThrowLastSocketError("Socket setup");
    }
}

void MockWebServerImpl::StartListening()
{
    if (listen(m_socket->Get(), 2 /*backlog*/) == SOCKET_ERROR)
    {
        ThrowLastSocketError("Socket listen");
    }

    m_listenerThread = std::thread(&MockWebServerImpl::StaticListen, std::ref(*this));
}

void MockWebServerImpl::StaticListen(MockWebServerImpl& server)
{
    try
    {
        server.Listen();
    }
    catch (const std::bad_alloc&)
    {
        server.m_listenerThreadResult = Result(Result::E_OutOfMemory);
    }
    catch (const SFSException& e)
    {
        server.m_listenerThreadResult = e.GetResult();
    }
    catch (...)
    {
        server.m_listenerThreadResult = Result(Result::E_Unexpected);
    }

    // Closing the socket here covers both a normal shutdown and an exception.
    {
        std::lock_guard<std::mutex> guard(server.m_socketMutex);
        server.m_socket.reset();
    }
}

void MockWebServerImpl::Listen()
{
    UNSCOPED_INFO("Listening on " << GetUrl());

    timeval timeout = {0, 100000}; // 0.1 sec = 0 sec, 100000 microseconds

    fd_set acceptSet;
    while (true)
    {
        // First, see if we are shutting down.
        {
            std::lock_guard<std::mutex> guard(m_shutdownMutex);
            if (m_shutdown)
            {
                break;
            }
        }

        // Now, wait for an incoming connection on the listening socket.

        // Set up the fd_set for the select call. The only file descriptor we are interested in is the listening socket.
        FD_ZERO(&acceptSet);
        FD_SET(m_socket->Get(), &acceptSet);

#ifdef _WIN32
        // On Windows, the first argument to select() is ignored.
        int nfds = 1;
#else
        // On Linux, the first argument to select() has to be the highest-numbered file descriptor in any of the sets,
        // plus 1.
        int nfds = m_socket->Get() + 1;
#endif

        // Wait for a connection until the timeout expires
        const int selectRet =
            select(nfds, &acceptSet /*readfds*/, nullptr /*writefds*/, nullptr /*exceptfds*/, &timeout);
        const bool timeoutReached = selectRet == 0;
        if (timeoutReached)
        {
            continue;
        }
        else if (selectRet == SOCKET_ERROR)
        {
            ThrowLastSocketError("Socket select");
        }

        // accept() will not block since select() said one socket was ready.
        sockaddr addr;
        socket_length_t addrlen = sizeof(addr);

        std::lock_guard<std::mutex> guard(m_socketMutex);
        if (m_socket->Get() != INVALID_SOCKET)
        {
            // Get the socket from the client who is trying to connect
            ScopedSocket clientSocket(accept(m_socket->Get(), &addr, &addrlen));
            try
            {
                ProcessRequest(clientSocket); // calls ::shutdown on send/recv as needed.
            }
            catch (const SFSException& e)
            {
                shutdown(clientSocket.Get(), SD_BOTH);
                UNSCOPED_INFO("Exception processing request: " << e.what());
                throw e;
            }
        }
    }
}

void MockWebServerImpl::ProcessRequest(ScopedSocket& clientSocket) const
{
    UNSCOPED_INFO("Received a request, processing it");

    // Read the request.
    std::unique_ptr<HttpRequest> request;
    ReceiveRequest(clientSocket, request);

    // Done reading request so disable the read half of connection.
    shutdown(clientSocket.Get(), SD_RECEIVE);

    UNSCOPED_INFO("Request: \n" << request->ToString());

    // Now create a response and send it
    HttpResponse response;
    GenerateResponse(*request, response);
    std::string responseStr = response.ToTransportString();

    UNSCOPED_INFO("Response: \n" << responseStr);

    SendResponse(clientSocket, responseStr);

    // Now disable the write half of connection.
    shutdown(clientSocket.Get(), SD_SEND);
}

void MockWebServerImpl::ReceiveRequest(ScopedSocket& clientSocket, std::unique_ptr<HttpRequest>& request) const
{
    const int bufSize = 16384;
    char buffer[bufSize];
    std::string data;

    int bytesReceived;
    do
    {
        bytesReceived = recv(clientSocket.Get(), buffer, bufSize, 0 /*flags*/);
        if (bytesReceived > 0)
        {
            data.append(buffer, bytesReceived);
        }
        else if (bytesReceived < 0)
        {
            ThrowLastSocketError("Socket receive");
        }

        // Check if we have a complete request and then break out of the loop.
        try
        {
            // This will throw if the request is not yet parseable. The body may not be complete yet,
            // so we will keep reading until we get a complete request.
            request = std::make_unique<HttpRequest>(data);
            break;
        }
        catch (const SFSException& e)
        {
            UNSCOPED_INFO("Incomplete request, keep reading. Parse issue: " << e.what());
        }
    } while (bytesReceived > 0);
}

void MockWebServerImpl::GenerateResponse(const HttpRequest& request, HttpResponse& response) const
{
    std::optional<std::string> version;
    std::string name;
    ApiMethod apiMethod;

    try
    {
        apiMethod = GetApiMethod(request, version, name);
    }
    catch (const HttpException& e)
    {
        response.SetStatus(e.GetStatusCode());
        return;
    }

    auto it = m_products.find(name);
    if (it == m_products.end())
    {
        response.SetStatus(StatusCode::NotFound);
        return;
    }

    const VersionList& versions = it->second;
    if (versions.empty())
    {
        response.SetStatus(StatusCode::InternalServerError);
        return;
    }

    json jsonResponse;
    switch (apiMethod)
    {
    case ApiMethod::GetSpecificVersion:
    {
        if (!version || !versions.count(*version))
        {
            response.SetStatus(StatusCode::NotFound);
            return;
        }
        GenerateGetSpecificVersionResponse(name, *version, jsonResponse);
        break;
    }
    case ApiMethod::PostLatestVersion:
    {
        const auto& latestVersion = *versions.rbegin();
        GeneratePostLatestVersionResponse(name, latestVersion, jsonResponse);
        break;
    }
    case ApiMethod::PostDownloadInfo:
    {
        if (!version || !versions.count(*version))
        {
            response.SetStatus(StatusCode::NotFound);
            return;
        }
        GeneratePostDownloadInfo(name, jsonResponse);
        break;
    }
    default:
        break;
    }

    response.SetBody(jsonResponse.dump());
}

void MockWebServerImpl::SendResponse(ScopedSocket& clientSocket, const std::string& response) const
{
    int totalBytesSent = 0;
    int bytesSent = 0;
    int bytesRemaining = static_cast<int>(response.size());
    const char* buffer = response.data();
    do
    {
        bytesSent = send(clientSocket.Get(), buffer + totalBytesSent, bytesRemaining, 0);
        if (bytesSent == SOCKET_ERROR)
        {
            ThrowLastSocketError("Socket send");
        }
        totalBytesSent += bytesSent;
        bytesRemaining -= bytesSent;
    } while (bytesRemaining > 0);
}

Result MockWebServerImpl::Stop()
{
    if (!m_isRunning)
    {
        return m_listenerThreadResult.value_or(Result::S_Ok);
    }

    {
        std::lock_guard<std::mutex> guard(m_shutdownMutex);
        m_shutdown = true;
    }
    if (m_listenerThread.joinable())
    {
        m_listenerThread.join();
    }
#ifdef _WIN32
    WSACleanup();
#endif
    m_isRunning = false;

    return m_listenerThreadResult.value_or(Result::S_Ok);
}

std::string MockWebServerImpl::GetUrl() const
{
    return c_listenHostName + std::string(":") + GetBindPort();
}

std::string MockWebServerImpl::GetBindPort() const
{
    sockaddr_in addr{};
    socket_length_t sizeAddr = sizeof(addr);

    if (getsockname(m_socket->Get(), reinterpret_cast<sockaddr*>(&addr), &sizeAddr))
    {
        ThrowLastSocketError("Socket getsockname");
    }

    return std::to_string(ntohs(addr.sin_port));
}

ScopedSocket::ScopedSocket()
{
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET)
    {
        ThrowLastSocketError("Socket creation");
    }
}

ScopedSocket::ScopedSocket(SOCKET socket)
{
    if (socket == INVALID_SOCKET)
    {
        ThrowSocketError("Invalid socket");
    }
    m_socket = socket;
}

ScopedSocket::~ScopedSocket()
{
    if (m_socket != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(m_socket);
#else
        close(m_socket);
#endif
        m_socket = INVALID_SOCKET;
    }
}

void MockWebServerImpl::RegisterProduct(const std::string& name, const std::string& version)
{
    m_products[name].insert(version);
}

ApiMethod MockWebServerImpl::GetApiMethod(const HttpRequest& request,
                                          std::optional<std::string>& version,
                                          std::string& name) const
{
    // Path example: /api/v1/contents/default/namespaces/default/names/ACDC.ACDUALTest07/versions/<version>
    // Arguments must be in order
    // SFS throws 404 NotFound when the URL is malformed

    std::string_view path = request.GetPath();
    if (path.empty() || path[0] != '/')
    {
        throw HttpException(StatusCode::BadRequest);
    }

    std::size_t offset = 1;
    auto getNextWord([&]() -> std::string_view {
        if (offset >= path.size())
        {
            throw HttpException(StatusCode::NotFound);
        }
        std::size_t nextSlash = path.find('/', offset);
        std::string_view word;
        if (nextSlash != std::string::npos)
        {
            word = path.substr(offset, nextSlash - offset);
        }
        else
        {
            word = path.substr(offset);
        }
        offset += word.size() + 1;
        return word;
    });

    std::string_view word = getNextWord();
    if (AreNotEqualI(word, "api"))
    {
        throw HttpException(StatusCode::NotFound);
    }
    std::string_view api = getNextWord();

    word = getNextWord();
    if (AreNotEqualI(word, "contents"))
    {
        throw HttpException(StatusCode::NotFound);
    }
    std::string_view tmp = getNextWord(); // Currently ignored

    word = getNextWord();
    if (AreNotEqualI(word, "namespaces"))
    {
        throw HttpException(StatusCode::NotFound);
    }
    tmp = getNextWord(); // Currently ignored

    word = getNextWord();
    if (AreEqualI(word, "names?action=BatchUpdates"))
    {
        // Path: /api/<apiVersion:v2>/contents/<instanceId>/namespaces/<ns>/names?action=BatchUpdates
        const bool isPostLatestVersion =
            !version.has_value() && request.GetMethod() == Method::POST && AreEqualI(api, "v2");
        if (isPostLatestVersion)
        {
            json body;
            try
            {
                body = json::parse(request.GetBody());
            }
            catch (const json::parse_error& ex)
            {
                UNSCOPED_INFO("JSON parse error: " << ex.what());
                throw HttpException(StatusCode::BadRequest);
            }

            // The BatchUpdates API returns an array of objects, each with a "Product" key.
            // For this mock we're only interested in the first one.
            // In the future we may want to support multiple products in a single request.
            if (body.is_array() && body[0].is_object() && body[0].contains("Product"))
            {
                name = body[0]["Product"];
            }
            else
            {
                throw HttpException(StatusCode::BadRequest);
            }

            return ApiMethod::PostLatestVersion;
        }
        throw HttpException(StatusCode::NotFound);

        // TODO: SFS throws a different error when the query string is unexpected
    }
    else if (AreNotEqualI(word, "names"))
    {
        throw HttpException(StatusCode::NotFound);
    }
    name = getNextWord();

    word = getNextWord();
    if (AreNotEqualI(word, "versions"))
    {
        throw HttpException(StatusCode::NotFound);
    }
    version = getNextWord();

    if (offset >= path.size())
    {
        // Path: /api/<apiVersion:v1>/contents/<instanceId>/namespaces/<ns>/names/<name>/versions/<version>
        const bool isGetSpecificVersion =
            version && !version->empty() && request.GetMethod() == Method::GET && AreEqualI(api, "v1");
        if (isGetSpecificVersion)
        {
            return ApiMethod::GetSpecificVersion;
        }
        throw HttpException(StatusCode::NotFound);
    }

    word = getNextWord();
    if (AreEqualI(word, "files?action=GenerateDownloadInfo"))
    {
        // Path:
        // /api/<apiVersion:v1>/contents/<instanceId>/namespaces/<ns>/names/<name>/versions/<version>/files?action=GenerateDownloadInfo
        const bool isPostDownloadInfo =
            version && !version->empty() && request.GetMethod() == Method::POST && AreEqualI(api, "v1");
        if (isPostDownloadInfo)
        {
            return ApiMethod::PostDownloadInfo;
        }

        // TODO: SFS throws a different error when the query string is unexpected
    }

    throw HttpException(StatusCode::NotFound);
}
