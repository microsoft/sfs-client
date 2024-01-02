// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Connection.h"

using namespace SFS;
using namespace SFS::details;

Connection::Connection(const ReportingHandler& handler) : m_handler(handler)
{
}

Result Connection::Post(std::string_view url, std::string& response)
{
    return Post(url, {}, response);
}
