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
        S_Ok = 0x00000000,
        E_NotImpl = 0x80000001,
        E_NotSet = 0x80000002,
        E_OutOfMemory = 0x80000003,
        E_Unexpected = 0x80000004,
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

  private:
    Code m_code;
    std::string m_message;
};

std::string_view ToString(Result::Code code) noexcept;
} // namespace SFS
