// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "SFSClientInterface.h"

#include "Content.h"
#include "Logging.h"
#include "Result.h"

#include <memory>
#include <optional>
#include <string>

namespace SFS::details
{
class DownloadInfoResponse;
class VersionResponse;

class SFSClientImpl : public SFSClientInterface
{
  public:
    SFSClientImpl(std::string&& accountId, std::string&& instanceId, std::string&& nameSpace);
    ~SFSClientImpl() = default;

    //
    // Individual APIs 1:1 with service endpoints (SFSClientInterface)
    //

    /**
     * @brief Gets the metadata for the latest available version for the specified product that matches the optional
     * request attributes
     */
    [[nodiscard]] Result GetLatestVersion(std::string_view productName,
                                          const std::optional<SearchAttributes>& attributes,
                                          std::unique_ptr<VersionResponse>& response) const override;

    /**
     * @brief Gets the metadata for a specific version of the specified product
     */
    [[nodiscard]] Result GetSpecificVersion(std::string_view productName,
                                            std::string_view version,
                                            const std::optional<SearchAttributes>& attributes,
                                            std::unique_ptr<VersionResponse>& content) const override;

    /**
     * @brief Gets the files metadata for a specific version of the specified product
     */
    [[nodiscard]] Result GetDownloadInfo(std::string_view productName,
                                         std::string_view version,
                                         const std::optional<SearchAttributes>& attributes,
                                         std::unique_ptr<DownloadInfoResponse>& content) const override;

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

    /**
     * @brief Set a logging callback function that is called when the SFSClient logs a message.
     */
    void SetLoggingCallback(LoggingCallbackFn&& callback);

  private:
    std::string m_accountId;
    std::string m_instanceId;
    std::string m_nameSpace;

    std::optional<std::string> m_customBaseUrl;
};
} // namespace SFS::details
