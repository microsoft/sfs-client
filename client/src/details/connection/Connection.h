// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../CorrelationVector.h"
#include "ConnectionConfig.h"

#include <string>

namespace SFS::details
{
class ReportingHandler;

class Connection
{
  public:
    Connection(const ReportingHandler& handler);

    virtual ~Connection()
    {
    }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    /**
     * @brief Perform a GET request to the given @param url
     * @return The response body
     * @throws SFSException if the request fails
     */
    virtual std::string Get(const std::string& url) = 0;

    /**
     * @brief Perform a POST request to the given @param url with @param data as the request body
     * @return The response body
     * @throws SFSException if the request fails
     */
    virtual std::string Post(const std::string& url, const std::string& data) = 0;

    /**
     * @brief Perform a POST request to the given @param url
     * @return The response body
     * @throws SFSException if the request fails
     */
    std::string Post(const std::string& url);

    /**
     * @brief Set the correlation vector to use for requests
     */
    void SetCorrelationVector(const std::string& cv);

    /**
     * @brief Set the connection configuration @param config
     */
    void SetConfig(ConnectionConfig config);

  protected:
    const ReportingHandler& m_handler;

    /// @brief The correlation vector to use for requests
    CorrelationVector m_cv;

    /// @brief Set of configurations for this connection
    ConnectionConfig m_config{};
};
} // namespace SFS::details
