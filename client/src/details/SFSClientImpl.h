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
    /**
     * @brief Make a new SFSClientImpl object
     */
    [[nodiscard]] static Result Make(ClientConfig&& config, std::unique_ptr<SFSClientInterface>& out);

    ~SFSClientImpl() override = default;

    //
    // Individual APIs 1:1 with service endpoints (SFSClientInterface)
    //

    /**
     * @brief Gets the metadata for the latest available version for the specified product that matches the optional
     * request attributes
     */
    [[nodiscard]] Result GetLatestVersion(const std::string& productName,
                                          const SearchAttributes& attributes,
                                          Connection& connection,
                                          std::unique_ptr<ContentId>& contentId) const override;

    /**
     * @brief Gets the metadata for a specific version of the specified product
     */
    [[nodiscard]] Result GetSpecificVersion(const std::string& productName,
                                            const std::string& version,
                                            Connection& connection,
                                            std::unique_ptr<ContentId>& contentId) const override;

    /**
     * @brief Gets the files metadata for a specific version of the specified product
     */
    [[nodiscard]] Result GetDownloadInfo(const std::string& productName,
                                         const std::string& version,
                                         Connection& connection,
                                         std::vector<std::unique_ptr<File>>& files) const override;

    /**
     * @brief Returns the ConnectionManager to be used by the SFSClient to create Connection objects
     */
    ConnectionManager& GetConnectionManager() override;

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
    SFSClientImpl() = default;

    std::string m_accountId;
    std::string m_instanceId;
    std::string m_nameSpace;

    std::unique_ptr<ConnectionManagerT> m_connectionManager;

    std::optional<std::string> m_customBaseUrl;
};
} // namespace SFS::details
