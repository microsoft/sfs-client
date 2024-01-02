// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace SFS::test
{
namespace details
{
enum class Method
{
    GET,
    POST
};

class HttpRequest
{
  public:
    HttpRequest(const std::string& data);

    Method GetMethod() const;
    const std::string& GetPath() const;
    std::string GetHeaders() const;
    const std::string& GetBody() const;

    std::string ToString() const;

  private:
    void Parse(const std::string& data);
    void ParseHeaders(std::string_view headers);

    bool IsComplete();
    std::optional<int> GetContentLength();

    Method m_method;
    std::string m_path;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
};

enum class StatusCode
{
    Ok = 200,
    BadRequest = 400,
    NotFound = 404,
    MethodNotAllowed = 405,
    InternalServerError = 500,
    ServiceUnavailable = 503,
};

class HttpResponse
{
  public:
    HttpResponse() = default;
    ~HttpResponse() = default;

    HttpResponse(const HttpResponse&) = delete;
    HttpResponse& operator=(const HttpResponse&) = delete;

    void SetStatus(StatusCode statusCode);

    void SetBody(const std::string& body);

    std::string ToTransportString() const;

  private:
    StatusCode m_status{StatusCode::Ok};
    std::string m_body;
};

class HttpException
{
  public:
    HttpException(StatusCode statusCode);
    StatusCode GetStatusCode() const;

  private:
    StatusCode m_statusCode;
};
} // namespace details
} // namespace SFS::test
