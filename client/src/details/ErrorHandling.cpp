// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ErrorHandling.h"

#include "ReportingHandler.h"

using namespace SFS::details;

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
