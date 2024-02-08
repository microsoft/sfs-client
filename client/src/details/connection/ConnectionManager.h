// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"

#include <memory>

namespace SFS::details
{
class Connection;
class ReportingHandler;

class ConnectionManager
{
  public:
    virtual ~ConnectionManager();

    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    [[nodiscard]] virtual Result MakeConnection(std::unique_ptr<Connection>& out) = 0;

  protected:
    ConnectionManager(const ReportingHandler& handler);

    const ReportingHandler& m_handler;
};
} // namespace SFS::details
