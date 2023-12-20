// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace SFS::details
{
class ReportingHandler;

class Connection
{
  public:
    Connection(const ReportingHandler& handler);

    virtual ~Connection()
    {
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

  protected:
    const ReportingHandler& m_handler;
};

class CurlConnection : public Connection
{
  public:
    CurlConnection(const ReportingHandler& handler);
    ~CurlConnection() override;

  private:
    // TODO: curl handle
};
} // namespace SFS::details
