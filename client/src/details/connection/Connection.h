// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"

#include <string>
#include <string_view>

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

    [[nodiscard]] virtual Result Get(std::string_view url, std::string& response) = 0;
    [[nodiscard]] virtual Result Post(std::string_view url, std::string_view data, std::string& response) = 0;

    [[nodiscard]] Result Post(std::string_view url, std::string& response);

  protected:
    const ReportingHandler& m_handler;
};
} // namespace SFS::details
