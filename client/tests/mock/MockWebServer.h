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

    // Registers a product with the server. Will fill the other data with gibberish for testing purposes.
    void RegisterProduct(const std::string& name, const std::string& version);

  private:
    std::unique_ptr<details::MockWebServerImpl> m_impl;
};
} // namespace SFS::test
