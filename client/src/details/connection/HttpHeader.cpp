// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HttpHeader.h"

#include <correlation_vector/correlation_vector.h>

constexpr const char* c_userAgent = "Microsoft-SFSClient/" SFS_VERSION;

std::string SFS::details::ToString(HttpHeader header)
{
    switch (header)
    {
    case HttpHeader::ContentType:
        return "Content-Type";
    case HttpHeader::MSCV:
        return microsoft::correlation_vector::HEADER_NAME;
    case HttpHeader::RetryAfter:
        return "Retry-After";
    case HttpHeader::UserAgent:
        return "User-Agent";
    }

    return "";
}

std::string SFS::details::GetUserAgentValue()
{
    return c_userAgent;
}
