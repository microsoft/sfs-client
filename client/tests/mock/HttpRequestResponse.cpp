// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HttpRequestResponse.h"

#include "ErrorHandling.h"

#include <sstream>

using namespace SFS::details;
using namespace SFS::test::details;

namespace
{
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

std::string ToString(Method method)
{
    return method == Method::GET ? "GET" : "POST";
}
} // namespace

HttpRequest::HttpRequest(const std::string& data)
{
    Parse(data);
}

void HttpRequest::Parse(const std::string& data)
{
    // Usual format of an HTTP request is:
    // GET /path HTTP/1.1
    // Host: hostname
    // Content-Length: 123
    //
    // body
    //

    const size_t firstSpace = data.find(' ');
    if (firstSpace == std::string::npos)
    {
        throw SFSException(Result::E_Unexpected, "Unexpected first line");
    }

    std::string method = data.substr(0, firstSpace);
    if (method == "GET")
    {
        m_method = Method::GET;
    }
    else if (method == "POST")
    {
        m_method = Method::POST;
    }
    else
    {
        throw SFSException(Result::E_Unexpected, "Unexpected HTTP method " + method);
    }

    const size_t secondSpace = data.find(' ', firstSpace + 1);
    if (secondSpace == std::string::npos)
    {
        throw SFSException(Result::E_Unexpected, "Unexpected first line while searching for path");
    }

    m_path = data.substr(firstSpace + 1, secondSpace - firstSpace - 1);

    // Between end of first line and the body there should be a double CRLF, separating the headers from the body.
    const size_t endOfFirstLine = data.find("\r\n", secondSpace + 2);
    if (endOfFirstLine == std::string::npos)
    {
        throw SFSException(Result::E_Unexpected, "Could not find end of first line");
    }

    const size_t doubleCrLf = data.find("\r\n\r\n", endOfFirstLine + 2);
    if (doubleCrLf == std::string::npos)
    {
        throw SFSException(Result::E_Unexpected, "Could not find double CRLF after headers");
    }

    ParseHeaders(data.substr(endOfFirstLine + 2, doubleCrLf - endOfFirstLine));

    m_body = data.substr(doubleCrLf + 4);

    if (!IsComplete())
    {
        throw SFSException(Result::E_Unexpected, "Incomplete HTTP request");
    }
}

void HttpRequest::ParseHeaders(std::string_view headers)
{
    size_t begin = 0;
    while (begin < headers.size())
    {
        const size_t lineEnd = headers.find("\r\n", begin);
        if (lineEnd == std::string::npos)
        {
            throw SFSException(Result::E_Unexpected, "Could not find end of line while parsing headers");
        }

        std::string_view header = headers.substr(begin, lineEnd - begin);
        const size_t colon = header.find(':');
        if (colon == std::string::npos)
        {
            throw SFSException(Result::E_Unexpected, "Could not find colon while parsing headers");
        }

        std::string headerName = std::string(header.substr(0, colon));
        std::string headerValue = std::string(header.substr(colon + 2)); // Skip the colon and the space after it

        m_headers.emplace(std::move(headerName), std::move(headerValue));

        begin = lineEnd + 2;
    }
}

Method HttpRequest::GetMethod() const
{
    return m_method;
}

const std::string& HttpRequest::GetPath() const
{
    return m_path;
}

std::string HttpRequest::GetHeaders() const
{
    std::stringstream ss;
    bool first = true;
    for (const auto& [headerName, headerValue] : m_headers)
    {
        if (!first)
        {
            ss << "; ";
        }
        ss << headerName << ": " << headerValue;
        first = false;
    }

    return ss.str();
}

const std::string& HttpRequest::GetBody() const
{
    return m_body;
}

std::optional<int> HttpRequest::GetContentLength()
{
    auto it = m_headers.find("Content-Length");
    if (it != m_headers.end())
    {
        return std::stoi(it->second);
    }
    return std::nullopt;
}

bool HttpRequest::IsComplete()
{
    // If there is a Content-Length header, then we can check if we have received all the body.
    if (auto contentLength = GetContentLength())
    {
        return static_cast<int>(m_body.size()) >= contentLength.value();
    }

    // Otherwise, we can only check if we have received a double CRLF and an empty body
    return m_body.empty();
}

std::string HttpRequest::ToString() const
{
    std::stringstream ss;
    ss << "Method: [" << ::ToString(GetMethod()) << "]" << std::endl;
    ss << "Path: [" << GetPath() << "]" << std::endl;
    ss << "Headers: [" << GetHeaders() << "]" << std::endl;
    ss << "Body: [" << GetBody() << "]" << std::endl;
    return ss.str();
}

void HttpResponse::SetBody(const std::string& body)
{
    m_body = body;
}

std::string HttpResponse::ToTransportString() const
{
    std::stringstream ss;
    ss << "HTTP/1.1 " << ::ToString(m_status) << std::endl;
    if (m_status == StatusCode::Ok)
    {
        ss << "Content-Type: application/json" << std::endl;
    }
    else
    {
        ss << "Content-Type: text/plain; charset=utf-8" << std::endl;
    }
    ss << "Content-Length: " << m_body.size() << std::endl;
    ss << std::endl;
    ss << m_body;
    return ss.str();
}

void HttpResponse::SetStatus(StatusCode statusCode)
{
    m_status = statusCode;
}

HttpException::HttpException(StatusCode statusCode) : m_statusCode(statusCode)
{
}

StatusCode HttpException::GetStatusCode() const
{
    return m_statusCode;
}
