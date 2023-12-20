// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>

namespace SFS::details
{
class Connection;
class ReportingHandler;

class ConnectionManager
{
  public:
    ConnectionManager(const ReportingHandler& handler);
    virtual ~ConnectionManager()
    {
    }

    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    virtual std::unique_ptr<Connection> MakeConnection() = 0;

  protected:
    const ReportingHandler& m_handler;
};

class CurlConnectionManager : public ConnectionManager
{
  public:
    CurlConnectionManager(const ReportingHandler& handler);
    ~CurlConnectionManager() override;

    std::unique_ptr<Connection> MakeConnection() override;
};
} // namespace SFS::details
