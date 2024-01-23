// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"

#include <exception>
#include <string>

namespace SFS::details
{
class SFSException : public std::exception
{
  public:
    SFSException() = default;
    SFSException(SFS::Result result);

    SFSException(SFS::Result::Code code, std::string message = {});

    const SFS::Result& GetResult() const noexcept;
    const char* what() const noexcept override;

  private:
    SFS::Result m_result;
};
} // namespace SFS::details
