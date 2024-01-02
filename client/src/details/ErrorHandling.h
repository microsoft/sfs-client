// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"
#include "SFSException.h"

#define SFS_CATCH_RETURN()                                                                                             \
    catch (const std::bad_alloc&)                                                                                      \
    {                                                                                                                  \
        return Result::E_OutOfMemory;                                                                                  \
    }                                                                                                                  \
    catch (const SFS::details::SFSException& e)                                                                        \
    {                                                                                                                  \
        return e.GetResult();                                                                                          \
    }                                                                                                                  \
    catch (const std::exception&)                                                                                      \
    {                                                                                                                  \
        return Result::E_Unexpected;                                                                                   \
    }                                                                                                                  \
    catch (...)                                                                                                        \
    {                                                                                                                  \
        return Result::E_Unexpected;                                                                                   \
    }

#define RETURN_IF_FAILED(result)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        auto __result = (result); /* Assigning to a variable ensures a code block gets called only once */             \
        if (__result.IsFailure())                                                                                      \
        {                                                                                                              \
            return __result;                                                                                           \
        }                                                                                                              \
    } while ((void)0, 0)

#define RETURN_IF_FAILED_LOG(result, handler)                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        auto __result = (result); /* Assigning to a variable ensures a code block gets called only once */             \
        if (__result.IsFailure())                                                                                      \
        {                                                                                                              \
            SFS::details::LogFailedResult(handler, __result, __FILE__, __LINE__);                                      \
            return __result;                                                                                           \
        }                                                                                                              \
    } while ((void)0, 0)

#define RETURN_CODE_IF_LOG(code, condition, handler, ...)                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (condition)                                                                                                 \
        {                                                                                                              \
            auto __result = SFS::Result(SFS::Result::code, ##__VA_ARGS__);                                             \
            SFS::details::LogFailedResult(handler, __result, __FILE__, __LINE__);                                      \
            return __result;                                                                                           \
        }                                                                                                              \
    } while ((void)0, 0)

#define THROW_CODE_IF_LOG(code, condition, handler, ...)                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (condition)                                                                                                 \
        {                                                                                                              \
            auto __result = SFS::Result(SFS::Result::code, ##__VA_ARGS__);                                             \
            SFS::details::LogFailedResult(handler, __result, __FILE__, __LINE__);                                      \
            throw SFS::details::SFSException(__result);                                                                \
        }                                                                                                              \
    } while ((void)0, 0)

namespace SFS::details
{
class ReportingHandler;

void LogFailedResult(const ReportingHandler& handler, const SFS::Result& result, const char* file, int line);
} // namespace SFS::details
