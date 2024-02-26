// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Connection.h"

#include <string>

// Forward declaration
typedef void CURL;

namespace SFS::details
{
struct CurlHeaderList;
class ReportingHandler;

class CurlConnection : public Connection
{
  public:
    CurlConnection(const ReportingHandler& handler);
    ~CurlConnection() override;

    /**
     * @brief Perform a GET request to the given @param url
     * @return The response body
     * @throws SFSException if the request fails
     */
    std::string Get(const std::string& url) override;

    /**
     * @brief Perform a POST request to the given @param url with @param data as the request body
     * @return The response body
     * @throws SFSException if the request fails
     */
    std::string Post(const std::string& url, const std::string& data) override;

  protected:
    /**
     * @brief Perform a REST request to the given @param url with the given @param headers
     * @return The response body
     * @throws SFSException if the request fails
     */
    virtual std::string CurlPerform(const std::string& url, CurlHeaderList& headers);

    CURL* m_handle;
};
} // namespace SFS::details
