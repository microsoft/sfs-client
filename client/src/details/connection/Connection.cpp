// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Connection.h"

using namespace SFS;
using namespace SFS::details;

Connection::Connection(const ReportingHandler& handler) : m_handler(handler)
{
}

Connection::~Connection()
{
}

Result Connection::Post(const std::string& url, std::string& response)
{
    return Post(url, {}, response);
}
