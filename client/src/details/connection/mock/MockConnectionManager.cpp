// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "MockConnectionManager.h"

#include "../../ErrorHandling.h"
#include "MockConnection.h"

using namespace SFS;
using namespace SFS::details;

MockConnectionManager::MockConnectionManager(const ReportingHandler& handler) : ConnectionManager(handler)
{
}

MockConnectionManager::~MockConnectionManager()
{
}

Result MockConnectionManager::MakeConnection(std::unique_ptr<Connection>& out)
{
    RETURN_IF_FAILED_LOG(MockConnection::Make(m_handler, out), m_handler);
    return Result::Success;
}
