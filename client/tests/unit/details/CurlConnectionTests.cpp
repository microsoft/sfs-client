// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../util/TestHelper.h"
#include "ReportingHandler.h"
#include "connection/CurlConnection.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#define TEST(...) TEST_CASE("[CurlConnectionTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details;
using namespace SFS::test;
using json = nlohmann::json;

namespace
{
class MockCurlConnection : public CurlConnection
{
  public:
    MockCurlConnection(const ReportingHandler& handler, Result::Code& responseCode, std::string& response)
        : CurlConnection(handler)
        , m_responseCode(responseCode)
        , m_response(response)
    {
    }

  protected:
    [[nodiscard]] Result CurlPerform(std::string_view, std::string& response) override
    {
        if (m_responseCode == Result::S_Ok)
        {
            response = m_response;
        }
        return m_responseCode;
    }

  private:
    Result::Code& m_responseCode;
    std::string& m_response;
};
} // namespace

TEST("Testing CurlConnection()")
{
    ReportingHandler handler;
    handler.SetLoggingCallback(LogCallbackToTest);
    Result::Code responseCode = Result::E_HttpNotFound;
    std::string response = "expected";
    std::unique_ptr<Connection> connection = std::make_unique<MockCurlConnection>(handler, responseCode, response);

    SECTION("Testing CurlConnection::Get()")
    {
        std::string out;
        REQUIRE(connection->Get("url", out) == responseCode);
        REQUIRE(out.empty());

        responseCode = Result::S_Ok;
        REQUIRE(connection->Get("url", out) == responseCode);
        REQUIRE(out == response);
    }

    SECTION("Testing CurlConnection::Post()")
    {
        std::string out;
        REQUIRE(connection->Post("url", out) == responseCode);
        REQUIRE(out.empty());

        const json body = {{{"dummy", {}}}};
        REQUIRE(connection->Post("url", body.dump(), out) == responseCode);
        REQUIRE(out.empty());

        responseCode = Result::S_Ok;
        REQUIRE(connection->Get("url", out) == responseCode);
        REQUIRE(out == response);

        response.clear();
        REQUIRE(connection->Post("url", body.dump(), out) == responseCode);
        REQUIRE(out == response);
    }
}
