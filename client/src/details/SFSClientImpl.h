// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "SFSClientInterface.h"

#include "ClientConfig.h"
#include "Content.h"
#include "Logging.h"
#include "Result.h"

#include <memory>
#include <optional>
#include <string>

namespace SFS::details
{
template <typename ConnectionManagerT>
class SFSClientImpl : public SFSClientInterface
{
  public:
    SFSClientImpl(ClientConfig&& config);
    ~SFSClientImpl() override = default;

    //
    // Individual APIs 1:1 with service endpoints (SFSClientInterface)
    //

    /**
     * @brief Gets the metadata for the latest available version for the specified product request
     * @return Entity that describes the latest version of the product
     * @throws SFSException if the request fails
     */
    std::unique_ptr<VersionEntity> GetLatestVersion(const ProductRequest& productRequest,
                                                    Connection& connection) const override;

    /**
     * @brief Gets the metadata for the latest available version for the specified product requests
     * @return Vector of entities that describe the latest version of the products
     * @throws SFSException if the request fails
     */
    VersionEntities GetLatestVersionBatch(const std::vector<ProductRequest>& productRequests,
                                          Connection& connection) const override;

    /**
     * @brief Gets the metadata for a specific version of the specified product
     * @return Entity that describes the latest version of the product
     * @throws SFSException if the request fails
     */
    std::unique_ptr<VersionEntity> GetSpecificVersion(const std::string& product,
                                                      const std::string& version,
                                                      Connection& connection) const override;

    /**
     * @brief Gets the files metadata for a specific version of the specified product
     * @return Vector of File entities for the specific version of the product
     * @throws SFSException if the request fails
     */
    FileEntities GetDownloadInfo(const std::string& product,
                                 const std::string& version,
                                 Connection& connection) const override;

    /**
     * @brief Returns a new Connection to be used by the SFSClient to make requests
     * @param config Configurations for the connection object
     */
    std::unique_ptr<Connection> MakeConnection(const ConnectionConfig& config) override;

    //
    // Configuration methods
    //

    /**
     * @brief Allows one to override the base URL used to make calls to the SFS service
     * @details Not exposed to the user. Used for testing purposes only
     * @param customBaseUrl The custom base URL to use
     */
    void SetCustomBaseUrl(std::string customBaseUrl);

    /**
     * @return The URL for the SFS service based on the parameters passed to the constructor
     */
    std::string GetBaseUrl() const;

  private:
    std::string m_accountId;
    std::string m_instanceId;
    std::string m_nameSpace;

    std::unique_ptr<ConnectionManagerT> m_connectionManager;

    std::optional<std::string> m_customBaseUrl;
};
} // namespace SFS::details
