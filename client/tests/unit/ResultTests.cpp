// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/Result.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[ResultTests] " __VA_ARGS__)

using namespace SFS;

TEST("Testing Result() class methods")
{
    SECTION("Default constructor")
    {
        Result resultOk(Result::Code::S_Ok);

        REQUIRE(resultOk.GetCode() == Result::Code::S_Ok);
        REQUIRE(resultOk.GetMessage().empty());
        REQUIRE(resultOk.IsSuccess());
        REQUIRE_FALSE(resultOk.IsFailure());

        INFO("Comparison operators");
        REQUIRE(resultOk == Result::Code::S_Ok);
        REQUIRE(resultOk == Result::S_Ok);
        REQUIRE(resultOk != Result::Code::E_NotSet);
        REQUIRE(resultOk != Result::E_NotSet);
        REQUIRE_FALSE(resultOk == Result::E_NotSet);

        INFO("bool operator");
        REQUIRE(resultOk);
    }

    SECTION("Constructor with message")
    {
        Result resultUnexpected(Result::Code::E_Unexpected, "message");
        REQUIRE(resultUnexpected.GetCode() == Result::Code::E_Unexpected);
        REQUIRE(resultUnexpected.GetMessage() == "message");
        REQUIRE_FALSE(resultUnexpected.IsSuccess());
        REQUIRE(resultUnexpected.IsFailure());

        INFO("Comparison operators on constructor with message");
        REQUIRE(resultUnexpected == Result::Code::E_Unexpected);
        REQUIRE(resultUnexpected != Result::Code::E_NotSet);

        INFO("bool operator");
        REQUIRE_FALSE(resultUnexpected);
    }
}

TEST("Testing ToString(Result)")
{
    REQUIRE(SFS::ToString(Result::Code::S_Ok) == "S_Ok");
    REQUIRE(SFS::ToString(Result::Code::E_NotImpl) == "E_NotImpl");
    REQUIRE(SFS::ToString(Result::Code::E_NotSet) == "E_NotSet");
    REQUIRE(SFS::ToString(Result::Code::E_OutOfMemory) == "E_OutOfMemory");
    REQUIRE(SFS::ToString(Result::Code::E_Unexpected) == "E_Unexpected");
}
