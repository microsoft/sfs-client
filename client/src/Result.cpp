// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Result.h"

#include <ostream>

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
    return GetCode() == Success;
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
    // Represents a successful operation
    case Result::Success:
        return "Success";
    // Represents a failed operation
    case Result::ConnectionSetupFailed:
        return "ConnectionSetupFailed";
    case Result::ConnectionUnexpectedError:
        return "ConnectionUnexpectedError";
    case Result::HttpBadRequest:
        return "HttpBadRequest";
    case Result::HttpNotFound:
        return "HttpNotFound";
    case Result::HttpServiceNotAvailable:
        return "HttpServiceNotAvailable";
    case Result::HttpSSLVerificationError:
        return "HttpSSLVerificationError";
    case Result::HttpTimeout:
        return "HttpTimeout";
    case Result::HttpUnexpected:
        return "HttpUnexpected";
    case Result::InvalidArg:
        return "InvalidArg";
    case Result::NotImpl:
        return "NotImpl";
    case Result::NotSet:
        return "NotSet";
    case Result::OutOfMemory:
        return "OutOfMemory";
    case Result::ServiceInvalidResponse:
        return "ServiceInvalidResponse";
    case Result::Unexpected:
        return "Unexpected";
    }
    return "";
}

std::ostream& operator<<(std::ostream& os, const Result& result)
{
    os << ToString(result.GetCode());
    return os;
}

std::ostream& operator<<(std::ostream& os, const Result::Code& code)
{
    os << ToString(code);
    return os;
}
