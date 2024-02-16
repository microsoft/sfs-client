// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <unordered_set>

namespace SFS
{
enum class RetriableHttpError
{
    TooManyRequests = 429,     // Rate limiting
    InternalServerError = 500, // Can be triggered within server timeouts, network issue
    BadGateway = 502,          // Likely an issue with routing
    ServerBusy = 503,
    GatewayTimeout = 504
};

struct ConnectionConfig
{
    /**
     * @brief The max number of retries that can be done for a web request after a failed first attempt, for retriable
     * errors. This number must be 0 <= maxRetries <= 3
     */
    unsigned maxRetries = 3;

    /**
     * @brief The base delay in milliseconds between attempts for a web request if a Retry-After header is not present.
     * maxSecForRetryAfter is considered if the Retry-After interval is sent. This number must be 15000ms <
     * retryDelayMs <= 60000ms. The delay between attempts will be exponential with a factor of 2.
     */
    unsigned retryDelayMs = 15000;

    /**
     * @brief If a recommended Retry-After interval is sent by the server, it will be honored if the value is less than
     * or equal to this value. If the value is bigger than this value, no retry will be made. If no interval is sent,
     * retryDelayMs is used instead.
     */
    unsigned maxSecForRetryAfter = 60;

    /// @brief Set of errors for which a retry will happen. If empty, there are no retries
    std::unordered_set<RetriableHttpError> retriableHttpErrors = {RetriableHttpError::TooManyRequests,
                                                                  RetriableHttpError::InternalServerError,
                                                                  RetriableHttpError::BadGateway,
                                                                  RetriableHttpError::ServerBusy,
                                                                  RetriableHttpError::GatewayTimeout};
};
} // namespace SFS
