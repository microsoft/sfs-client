// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "details/ErrorHandling.h"
#include "sfsclient/Result.h"

#include <gtest/gtest.h>

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

Result TestSFS_ReturnIfFailed(const Result& result, const Result& ifNotFailed = Result::S_Ok)
{
    RETURN_IF_FAILED(result);
    return ifNotFailed;
}
} // namespace

TEST(ErrorHandlingTests, SfsCatchReturn)
{
    EXPECT_EQ(TestSFS_Catch_Return_bad_alloc().GetCode(), Result::E_OutOfMemory);
    EXPECT_EQ(TestSFS_Catch_Return_std_exception().GetCode(), Result::E_Unexpected);
    EXPECT_EQ(TestSFS_Catch_Return_unknown().GetCode(), Result::E_Unexpected);
}

TEST(ErrorHandlingTests, SfsReturnIfFailed)
{
    // Test that RETURN_IF_FAILED returns the result if it is a failure
    EXPECT_EQ(TestSFS_ReturnIfFailed(Result::Code::E_Unexpected).GetCode(), Result::Code::E_Unexpected);

    // Test that RETURN_IF_FAILED does not return if the result is a success
    EXPECT_EQ(TestSFS_ReturnIfFailed(Result::Code::S_Ok).GetCode(), Result::Code::S_Ok);
    EXPECT_EQ(TestSFS_ReturnIfFailed(Result::Code::S_Ok, Result::Code::E_Unexpected).GetCode(),
              Result::Code::E_Unexpected);
}
