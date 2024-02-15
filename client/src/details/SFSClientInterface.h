// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Content.h"
#include "Logging.h"
#include "ReportingHandler.h"

#include <memory>
#include <string>

namespace SFS::details
{
class Connection;
class ConnectionManager;

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
     * @brief Gets the metadata for the latest available version for the specified product that matches the optional
     * request attributes
     * @return ContentId for the latest version of the product
     * @throws SFSException if the request fails
     */
    virtual std::unique_ptr<ContentId> GetLatestVersion(const std::string& productName,
                                                        const SearchAttributes& attributes,
                                                        Connection& connection) const = 0;

    /**
     * @brief Gets the metadata for a specific version of the specified product
     * @return ContentId for the specific version of the product
     * @throws SFSException if the request fails
     */
    virtual std::unique_ptr<ContentId> GetSpecificVersion(const std::string& productName,
                                                          const std::string& version,
                                                          Connection& connection) const = 0;

    /**
     * @brief Gets the files metadata for a specific version of the specified product
     * @return vector of File objects for the specific version of the product
     * @throws SFSException if the request fails
     */
    virtual std::vector<File> GetDownloadInfo(const std::string& productName,
                                              const std::string& version,
                                              Connection& connection) const = 0;

    /**
     * @brief Returns the ConnectionManager to be used by the SFSClient to create Connection objects
     */
    virtual ConnectionManager& GetConnectionManager() = 0;

    const ReportingHandler& GetReportingHandler() const
    {
        return m_reportingHandler;
    }

  protected:
    ReportingHandler m_reportingHandler;
};
} // namespace SFS::details
