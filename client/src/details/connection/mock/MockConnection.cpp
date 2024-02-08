// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "MockConnection.h"

using namespace SFS;
using namespace SFS::details;

Result MockConnection::Make(const ReportingHandler& handler, std::unique_ptr<Connection>& out)
{
    auto tmp = std::unique_ptr<MockConnection>(new MockConnection(handler));
    out = std::move(tmp);
    return Result::Success;
}

MockConnection::MockConnection(const ReportingHandler& handler) : Connection(handler)
{
}

MockConnection::~MockConnection()
{
}

Result MockConnection::Get(const std::string&, std::string&)
{
    return Result::Success;
}

Result MockConnection::Post(const std::string&, const std::string&, std::string&)
{
    return Result::Success;
}
