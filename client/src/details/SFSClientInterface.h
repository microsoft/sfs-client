// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Content.h"
#include "Logging.h"
#include "ReportingHandler.h"
#include "Result.h"

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
     */
    [[nodiscard]] virtual Result GetLatestVersion(const std::string& productName,
                                                  const SearchAttributes& attributes,
                                                  Connection& connection,
                                                  std::unique_ptr<ContentId>& contentId) const = 0;

    /**
     * @brief Gets the metadata for a specific version of the specified product
     */
    [[nodiscard]] virtual Result GetSpecificVersion(const std::string& productName,
                                                    const std::string& version,
                                                    Connection& connection,
                                                    std::unique_ptr<ContentId>& contentId) const = 0;

    /**
     * @brief Gets the files metadata for a specific version of the specified product
     */
    [[nodiscard]] virtual Result GetDownloadInfo(const std::string& productName,
                                                 const std::string& version,
                                                 Connection& connection,
                                                 std::vector<std::unique_ptr<File>>& files) const = 0;

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
