// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "MockConnection.h"

using namespace SFS;
using namespace SFS::details;

MockConnection::MockConnection(const ReportingHandler& handler) : Connection(handler)
{
}

MockConnection::~MockConnection()
{
}

std::string MockConnection::Get(const std::string&)
{
    return {};
}

std::string MockConnection::Post(const std::string&, const std::string&)
{
    return {};
}
