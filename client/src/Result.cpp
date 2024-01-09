// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Result.h"

using namespace SFS;

Result::Result(Code code) noexcept : m_code(code)
{
}

Result::Result(Code code, std::string message) noexcept : Result(code)
{
    try
    {
        m_message = std::move(message);
    }
    catch (...)
    {
        // Ignore exception, leave message empty
    }
}

Result::Code Result::GetCode() const noexcept
{
    return m_code;
}

const std::string& Result::GetMessage() const noexcept
{
    return m_message;
}

bool Result::IsSuccess() const noexcept
{
    return GetCode() == S_Ok;
}

bool Result::IsFailure() const noexcept
{
    return !IsSuccess();
}

Result::operator bool() const noexcept
{
    return IsSuccess();
}

bool Result::operator==(Code resultCode) const noexcept
{
    return GetCode() == resultCode;
}

bool Result::operator!=(Code resultCode) const noexcept
{
    return GetCode() != resultCode;
}

std::string_view SFS::ToString(Result::Code code) noexcept
{
    switch (code)
    {
    case Result::S_Ok:
        return "S_Ok";
    case Result::E_NotImpl:
        return "E_NotImpl";
    case Result::E_NotSet:
        return "E_NotSet";
    case Result::E_OutOfMemory:
        return "E_OutOfMemory";
    case Result::E_Unexpected:
        return "E_Unexpected";
    }
    return "";
}
