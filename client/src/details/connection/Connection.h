// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"

#include <string>

namespace SFS::details
{
class ReportingHandler;

class Connection
{
  public:
    virtual ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    [[nodiscard]] virtual Result Get(const std::string& url, std::string& response) = 0;
    [[nodiscard]] virtual Result Post(const std::string& url, const std::string& data, std::string& response) = 0;

    [[nodiscard]] Result Post(const std::string& url, std::string& response);

  protected:
    Connection(const ReportingHandler& handler);

    const ReportingHandler& m_handler;
};
} // namespace SFS::details
