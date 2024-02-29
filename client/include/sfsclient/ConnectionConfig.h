// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <chrono>

namespace SFS
{
constexpr unsigned c_maxMaxRetries = 3;

struct ConnectionConfig
{
    /**
     * @brief The max number of retries that can be done for a web request after a failed first attempt, for retriable
     * errors. This number must be 0 <= maxRetries <= 3
     */
    unsigned maxRetries = 3;

    /**
     * @brief The max duration for a single web request made to the SFS API. This value is enforced only after the
     * service responds or times out.
     */
    std::chrono::milliseconds maxRequestDuration = std::chrono::minutes(2);
};
} // namespace SFS
