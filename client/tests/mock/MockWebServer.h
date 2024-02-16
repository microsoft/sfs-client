// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"

#include <memory>
#include <string>

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
    void RegisterExpectedHeader(std::string header, std::string value);

  private:
    std::unique_ptr<details::MockWebServerImpl> m_impl;
};
} // namespace SFS::test
