// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../Connection.h"
#include "Result.h"

#include <string>
#include <string_view>

namespace SFS::details
{
class ReportingHandler;

class MockConnection : public Connection
{
  public:
    MockConnection(const ReportingHandler& handler);
    ~MockConnection() override;

    [[nodiscard]] Result Get(std::string_view url, std::string& response) override;
    [[nodiscard]] Result Post(std::string_view url, std::string_view data, std::string& response) override;
};
} // namespace SFS::details
