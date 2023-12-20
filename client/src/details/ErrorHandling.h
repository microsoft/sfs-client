// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Result.h"

#include <exception>

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

namespace SFS::details
{
class SFSException : public std::exception
{
  public:
    SFSException() = default;
    SFSException(SFS::Result result) : m_result(std::move(result))
    {
    }

    const SFS::Result& GetResult() const noexcept
    {
        return m_result;
    }

    const char* what() const noexcept override
    {
        return m_result.GetMessage().c_str();
    }

  private:
    SFS::Result m_result;
};
} // namespace SFS::details
