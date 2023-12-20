// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Connection.h"

using namespace SFS::details;

Connection::Connection(const ReportingHandler& handler) : m_handler(handler)
{
}

CurlConnection::CurlConnection(const ReportingHandler& handler) : Connection(handler)
{
}

CurlConnection::~CurlConnection()
{
}
