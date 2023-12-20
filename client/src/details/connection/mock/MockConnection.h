// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../Connection.h"

namespace SFS::details
{
class ReportingHandler;

class MockConnection : public Connection
{
  public:
    MockConnection(const ReportingHandler& handler);
    ~MockConnection() override;
};
} // namespace SFS::details
