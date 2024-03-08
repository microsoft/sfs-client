// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Content.h"
#include "Logging.h"
#include "ReportingHandler.h"
#include "RequestParams.h"
#include "SFSEntities.h"

#include <memory>
#include <string>

namespace SFS::details
{
class Connection;
class ConnectionManager;
struct ConnectionConfig;

using VersionEntities = std::vector<std::unique_ptr<VersionEntity>>;

class SFSClientInterface
{
  public:
    virtual ~SFSClientInterface()
    {
    }

    //
    // Individual APIs 1:1 with service endpoints
    //

    /**
     * @brief Gets the metadata for the latest available version for the specified product request
     * @return Entity that describes the latest version of the product
     * @throws SFSException if the request fails
     */
    virtual std::unique_ptr<VersionEntity> GetLatestVersion(const ProductRequest& productRequest,
                                                            Connection& connection) const = 0;

    /**
     * @brief Gets the metadata for the latest available version for the specified product requests
     * @return Vector of entities that describe the latest version of the products
     * @throws SFSException if the request fails
     */
    virtual VersionEntities GetLatestVersionBatch(const std::vector<ProductRequest>& productRequests,
                                                  Connection& connection) const = 0;

    /**
     * @brief Gets the metadata for a specific version of the specified product
     * @return Entity that describes the latest version of the product
     * @throws SFSException if the request fails
     */
    virtual std::unique_ptr<VersionEntity> GetSpecificVersion(const std::string& product,
                                                              const std::string& version,
                                                              Connection& connection) const = 0;

    /**
     * @brief Gets the files metadata for a specific version of the specified product
     * @return Vector of File entities for the specific version of the product
     * @throws SFSException if the request fails
     */
    virtual FileEntities GetDownloadInfo(const std::string& product,
                                         const std::string& version,
                                         Connection& connection) const = 0;

    /**
     * @brief Returns a new Connection to be used by the SFSClient to make requests
     * @param config Configurations for the connection object
     */
    virtual std::unique_ptr<Connection> MakeConnection(const ConnectionConfig& config) = 0;

    const ReportingHandler& GetReportingHandler() const
    {
        return m_reportingHandler;
    }

  protected:
    ReportingHandler m_reportingHandler;
};
} // namespace SFS::details
