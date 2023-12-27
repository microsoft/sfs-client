// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#define SFS_CATCH_RETURN()                                                                                             \
    catch (const std::bad_alloc&)                                                                                      \
    {                                                                                                                  \
        return Result::E_OutOfMemory;                                                                                  \
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
