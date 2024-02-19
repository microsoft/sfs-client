// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"

#include <deque>
#include <memory>
#include <string>

namespace SFS::details
{
enum class HttpHeader;
}

namespace SFS::test
{
namespace details
{
class MockWebServerImpl;
}
class MockWebServer
{
  public:
    MockWebServer();
    ~MockWebServer();

    MockWebServer(const MockWebServer&) = delete;
    MockWebServer& operator=(const MockWebServer&) = delete;

    [[nodiscard]] Result Stop();

    std::string GetBaseUrl() const;

    /// @brief Registers a product with the server. Will fill the other data with gibberish for testing purposes
    void RegisterProduct(std::string name, std::string version);

    /// @brief Registers the expectation of a given header to the present in the request
    void RegisterExpectedRequestHeader(SFS::details::HttpHeader header, std::string value);

    /**
     * @brief Registers a sequence of HTTP error codes that will be sent by the server in the order in which they are
     * passed.
     */
    void SetForcedHttpErrors(std::deque<int> forcedErrors);

  private:
    std::unique_ptr<details::MockWebServerImpl> m_impl;
};
} // namespace SFS::test
