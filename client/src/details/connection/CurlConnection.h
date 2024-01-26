// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Connection.h"
#include "Result.h"

#include <string>
#include <string_view>

// Forward declarations
#define SFS_CURL_ERROR_SIZE 256 // Size will be checked against curl lib reqs in source
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
    [[nodiscard]] virtual Result CurlPerform(std::string_view url, std::string& response);

    CURL* m_handle;
    char m_errorBuffer[SFS_CURL_ERROR_SIZE];
};
} // namespace SFS::details
