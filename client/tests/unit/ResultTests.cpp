// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/Result.h"

#include <gtest/gtest.h>

using namespace SFS;

TEST(ResultTests, ClassMethods)
{
    // Default constructor
    Result resultOk(Result::Code::S_Ok);
    EXPECT_EQ(resultOk.GetCode(), Result::Code::S_Ok);
    EXPECT_TRUE(resultOk.GetMessage().empty());
    EXPECT_TRUE(resultOk.IsSuccess());
    EXPECT_FALSE(resultOk.IsFailure());

    // Comparison operators
    EXPECT_TRUE(resultOk == Result::Code::S_Ok);
    EXPECT_TRUE(resultOk != Result::Code::E_NotSet);

    // bool operator
    EXPECT_TRUE(resultOk);

    // Constructor with message
    Result resultUnexpected(Result::Code::E_Unexpected, "message");
    EXPECT_EQ(resultUnexpected.GetCode(), Result::Code::E_Unexpected);
    EXPECT_EQ(resultUnexpected.GetMessage(), "message");
    EXPECT_FALSE(resultUnexpected.IsSuccess());
    EXPECT_TRUE(resultUnexpected.IsFailure());

    // Comparison operators on constructor with message
    EXPECT_TRUE(resultUnexpected == Result::Code::E_Unexpected);
    EXPECT_TRUE(resultUnexpected != Result::Code::E_NotSet);

    // bool operator
    EXPECT_FALSE(resultUnexpected);
}

TEST(ResultTests, ToString)
{
    EXPECT_EQ(SFS::ToString(Result::Code::S_Ok), "S_Ok");
    EXPECT_EQ(SFS::ToString(Result::Code::E_NotImpl), "E_NotImpl");
    EXPECT_EQ(SFS::ToString(Result::Code::E_NotSet), "E_NotSet");
    EXPECT_EQ(SFS::ToString(Result::Code::E_OutOfMemory), "E_OutOfMemory");
    EXPECT_EQ(SFS::ToString(Result::Code::E_Unexpected), "E_Unexpected");
}
