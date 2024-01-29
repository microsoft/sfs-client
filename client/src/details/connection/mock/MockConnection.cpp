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

Result MockConnection::Get(const std::string&, std::string&)
{
    return Result::S_Ok;
}

Result MockConnection::Post(const std::string&, const std::string&, std::string&)
{
    return Result::S_Ok;
}
