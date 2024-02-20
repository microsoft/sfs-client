// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
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
     * @brief Set the caller's application ID @param callerApplicationId
     */
    void SetCallerApplicationId(std::string callerApplicationId);

  protected:
    const ReportingHandler& m_handler;

    /// @brief The caller's application ID. This is sent to the SFS Service via the User-Agent HTTP header.
    std::optional<std::string> m_callerApplicationId;
};
} // namespace SFS::details
