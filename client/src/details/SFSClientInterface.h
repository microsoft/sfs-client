// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Content.h"
#include "Logging.h"
#include "ReportingHandler.h"
#include "Result.h"

#include <memory>
#include <optional>
#include <string>

namespace SFS::details
{
class DownloadInfoResponse;
class VersionResponse;

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
    [[nodiscard]] virtual Result GetLatestVersion(std::string_view productName,
                                                  const std::optional<SearchAttributes>& attributes,
                                                  std::unique_ptr<VersionResponse>& response) const = 0;

    /**
     * @brief Gets the metadata for a specific version of the specified product
     */
    [[nodiscard]] virtual Result GetSpecificVersion(std::string_view productName,
                                                    std::string_view version,
                                                    const std::optional<SearchAttributes>& attributes,
                                                    std::unique_ptr<VersionResponse>& content) const = 0;

    /**
     * @brief Gets the files metadata for a specific version of the specified product
     */
    [[nodiscard]] virtual Result GetDownloadInfo(std::string_view productName,
                                                 std::string_view version,
                                                 const std::optional<SearchAttributes>& attributes,
                                                 std::unique_ptr<DownloadInfoResponse>& content) const = 0;

    /**
     * @brief Set a logging callback function that is called when the SFSClient logs a message.
     */
    virtual void SetLoggingCallback(LoggingCallbackFn&& callback) = 0;

    const ReportingHandler& GetReportingHandler() const
    {
        return m_reportingHandler;
    }

  protected:
    ReportingHandler m_reportingHandler;
};
} // namespace SFS::details
