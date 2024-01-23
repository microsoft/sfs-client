// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Connection.h"

namespace SFS::details
{
class ReportingHandler;

class CurlConnection : public Connection
{
  public:
    CurlConnection(const ReportingHandler& handler);
    ~CurlConnection() override;

  private:
    // TODO: curl handle
};
} // namespace SFS::details
