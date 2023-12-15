// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "SFSClientInterface.h"

#include "Content.h"
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
    // Misc methods
    //

    /**
     * @return The URL for the SFS service based on the parameters passed to the constructor
     */
    std::string BuildUrl() const;

  private:
    std::string m_accountId;
    std::string m_instanceId;
    std::string m_nameSpace;
};
} // namespace SFS::details
