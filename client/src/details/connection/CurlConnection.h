// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Connection.h"
#include "Result.h"

#include <string>
#include <string_view>

// Forward declarations
#define CURL_ERROR_SIZE 256
typedef void CURL;

namespace SFS::details
{
class ReportingHandler;

class CurlConnection : public Connection
{
  public:
    CurlConnection(const ReportingHandler& handler);
    ~CurlConnection() override;

    [[nodiscard]] Result Get(std::string_view url, std::string& response) override;
    [[nodiscard]] Result Post(std::string_view url, std::string_view data, std::string& response) override;

  protected:
    [[nodiscard]] Result CurlPerform(std::string_view url, std::string& response);

    CURL* m_handle;
    char m_errorBuffer[CURL_ERROR_SIZE];
};
} // namespace SFS::details
