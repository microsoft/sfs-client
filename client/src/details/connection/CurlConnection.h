// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Connection.h"
#include "Result.h"

#include <string>

// Forward declaration
typedef void CURL;

namespace SFS::details
{
class ReportingHandler;

class CurlConnection : public Connection
{
  public:
    CurlConnection(const ReportingHandler& handler);
    ~CurlConnection() override;

    [[nodiscard]] Result Get(const std::string& url, std::string& response) override;
    [[nodiscard]] Result Post(const std::string& url, const std::string& data, std::string& response) override;

  protected:
    [[nodiscard]] virtual Result CurlPerform(const std::string& url, std::string& response);

    CURL* m_handle;
};
} // namespace SFS::details
