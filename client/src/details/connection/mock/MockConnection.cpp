// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "MockConnection.h"

using namespace SFS::details;

MockConnection::MockConnection(const ReportingHandler& handler) : Connection(handler)
{
}

MockConnection::~MockConnection()
{
}
