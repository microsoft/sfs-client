// Copyright (c) Microsoft Corporation. All rights reserved.

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
        if (result.IsFailure())                                                                                        \
        {                                                                                                              \
            return result;                                                                                             \
        }                                                                                                              \
    } while ((void)0, 0)
