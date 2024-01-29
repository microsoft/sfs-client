// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../Connection.h"
#include "Result.h"

#include <string>

namespace SFS::details
{
class ReportingHandler;

class MockConnection : public Connection
{
  public:
    MockConnection(const ReportingHandler& handler);
    ~MockConnection() override;

    [[nodiscard]] Result Get(const std::string& url, std::string& response) override;
    [[nodiscard]] Result Post(const std::string& url, const std::string& data, std::string& response) override;
};
} // namespace SFS::details
