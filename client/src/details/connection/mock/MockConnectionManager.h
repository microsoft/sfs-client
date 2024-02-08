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
    [[nodiscard]] static Result Make(const ReportingHandler& handler, std::unique_ptr<ConnectionManager>& out);

    ~MockConnectionManager() override;

    [[nodiscard]] Result MakeConnection(std::unique_ptr<Connection>& out) override;

  private:
    MockConnectionManager(const ReportingHandler& handler);
};
} // namespace SFS::details
