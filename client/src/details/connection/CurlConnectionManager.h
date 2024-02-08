// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "ConnectionManager.h"

#include <memory>

namespace SFS::details
{
class Connection;
class ReportingHandler;

class CurlConnectionManager : public ConnectionManager
{
  public:
    [[nodiscard]] static Result Make(const ReportingHandler& handler, std::unique_ptr<ConnectionManager>& out);

    ~CurlConnectionManager() override;

    [[nodiscard]] Result MakeConnection(std::unique_ptr<Connection>& out) override;

  protected:
    CurlConnectionManager(const ReportingHandler& handler);

    [[nodiscard]] Result SetupCurl();
};
} // namespace SFS::details
