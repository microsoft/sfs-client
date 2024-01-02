// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ErrorHandling.h"

#include "ReportingHandler.h"

using namespace SFS::details;

SFSException::SFSException(SFS::Result result) : m_result(std::move(result))
{
}

SFSException::SFSException(SFS::Result::Code code, std::string message) : m_result(Result(code, std::move(message)))
{
}

const SFS::Result& SFSException::GetResult() const noexcept
{
    return m_result;
}

const char* SFSException::what() const noexcept
{
    return m_result.GetMessage().c_str();
}

void SFS::details::LogFailedResult(const ReportingHandler& handler,
                                   const SFS::Result& result,
                                   const char* file,
                                   int line)
{
    if (result.IsFailure())
    {
        LOG_ERROR(handler,
                  "FAILED [%s] %s%s(%s:%d)",
                  std::string(ToString(result.GetCode())).c_str(),
                  result.GetMessage().c_str(),
                  result.GetMessage().empty() ? "" : " ",
                  file,
                  line);
    }
}
