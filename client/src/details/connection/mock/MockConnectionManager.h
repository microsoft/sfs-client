// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../ConnectionManager.h"

#include <memory>

namespace SFS::details
{
class Connection;
class ReportingHandler;

class MockConnectionManager : public ConnectionManager
{
  public:
    MockConnectionManager(const ReportingHandler& handler);
    ~MockConnectionManager() override;

    std::unique_ptr<Connection> MakeConnection() override;
};
} // namespace SFS::details
