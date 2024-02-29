// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace SFS
{
class Result
{
  public:
    enum Code : uint32_t
    {
        // Represents a successful operation
        Success = 0x00000000,
        // Represents a failed operation
        ConnectionSetupFailed = 0x80000001,
        ConnectionUnexpectedError = 0x80000002,
        HttpBadRequest = 0x80000003,
        HttpNotFound = 0x80000004,
        HttpServiceNotAvailable = 0x80000005,
        HttpTimeout = 0x80000006,
        HttpTooManyRequests = 0x80000007,
        HttpUnexpected = 0x80000008,
        InvalidArg = 0x80000009,
        NotImpl = 0x8000000A,
        NotSet = 0x8000000B,
        OutOfMemory = 0x8000000C,
        ServiceInvalidResponse = 0x8000000D,
        Unexpected = 0x8000000E,
    };

    Result(Code code) noexcept;
    Result(Code code, std::string message) noexcept;

    Code GetCode() const noexcept;
    const std::string& GetMessage() const noexcept;

    bool IsSuccess() const noexcept;
    bool IsFailure() const noexcept;

    /**
     * @brief Returns true if the result code represents a successful operation.
     */
    operator bool() const noexcept;

    bool operator==(Code resultCode) const noexcept;
    bool operator!=(Code resultCode) const noexcept;

    // Results should not be compared directly
    bool operator==(Result) const = delete;
    bool operator!=(Result) const = delete;
    bool operator>(Result) const = delete;
    bool operator>=(Result) const = delete;
    bool operator<(Result) const = delete;
    bool operator<=(Result) const = delete;

  private:
    Code m_code;
    std::string m_message;
};

std::string_view ToString(Result::Code code) noexcept;
} // namespace SFS

std::ostream& operator<<(std::ostream& os, const SFS::Result& value);
std::ostream& operator<<(std::ostream& os, const SFS::Result::Code& value);
