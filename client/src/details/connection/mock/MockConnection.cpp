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

Result MockConnection::Get(std::string_view, std::string&)
{
    return Result::S_Ok;
}

Result MockConnection::Post(std::string_view, std::string_view, std::string&)
{
    return Result::S_Ok;
}
