// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ErrorHandling.h"
#include "sfsclient/Result.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[ErrorHandlingTests] " __VA_ARGS__)

using namespace SFS;

namespace
{
Result TestSFS_Catch_Return_bad_alloc()
try
{
    throw std::bad_alloc();
}
SFS_CATCH_RETURN();

class MyException : public std::exception
{
};

Result TestSFS_Catch_Return_std_exception()
try
{
    throw MyException();
}
SFS_CATCH_RETURN();

Result TestSFS_Catch_Return_unknown()
try
{
    throw std::error_code();
}
SFS_CATCH_RETURN();

Result TestSFS_ReturnIfFailed(const Result& result, const Result& ifNotFailed = Result::Success)
{
    RETURN_IF_FAILED(result);
    return ifNotFailed;
}
} // namespace

TEST("Testing ErrorHandling's SFS_CATCH_RETURN()")
{
    REQUIRE(TestSFS_Catch_Return_bad_alloc() == Result::OutOfMemory);
    REQUIRE(TestSFS_Catch_Return_std_exception() == Result::Unexpected);
    REQUIRE(TestSFS_Catch_Return_unknown() == Result::Unexpected);
}

TEST("Testing ErrorHandling's RETURN_IF_FAILED()")
{
    SECTION("Test that RETURN_IF_FAILED returns the result if it is a failure")
    {
        REQUIRE(TestSFS_ReturnIfFailed(Result::Code::Unexpected) == Result::Code::Unexpected);
    }

    SECTION("Test that RETURN_IF_FAILED does not return if the result is a success")
    {
        REQUIRE(TestSFS_ReturnIfFailed(Result::Code::Success) == Result::Code::Success);
        REQUIRE(TestSFS_ReturnIfFailed(Result::Code::Success, Result::Code::Unexpected) == Result::Code::Unexpected);
    }
}
