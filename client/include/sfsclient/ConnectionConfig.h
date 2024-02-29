// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <unordered_set>

namespace SFS
{
struct ConnectionConfig
{
    /**
     * @brief The max number of retries that can be done for a web request after a failed first attempt, for retriable
     * errors. This number must be 0 <= maxRetries <= 3
     */
    unsigned maxRetries = 3;

    /**
     * @brief The base delay in milliseconds between attempts for a web request if a Retry-After header is not present.
     * This number must be 15000ms < retryDelayMs <= 60000ms. The delay between attempts will be exponential with a
     * factor of 2.
     */
    unsigned retryDelayMs = 15000;
};
} // namespace SFS
