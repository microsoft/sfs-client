// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Content.h"
#include "Logging.h"
#include "Result.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace SFS
{
class ApplicabilityDetails;
class DeliveryOptimizationData;

using Contents = std::vector<std::unique_ptr<Content>>;

namespace details
{
class SFSClientInterface;
}

struct Options
{
    /// @brief The account ID of the SFS service is used to identify the caller
    std::string accountId;

    /// @brief The instance ID of the SFS service
    std::optional<std::string> instanceId;

    /// @brief The namespace of the SFS service
    std::optional<std::string> nameSpace;

    /**
     * @brief A logging callback function that is called when the SFSClient logs a message
     * @details This function returns logging information from the SFSClient. The caller is responsible for incoporating
     * the received data into their logging system. The callback will be called in the same thread as the
     * main flow, so make sure the callback does not block for too long so it doesn't delay operations. The
     * LogData does not exist after the callback returns, so caller has to copy it if the data will be stored.
     */
    std::optional<LoggingCallbackFn> logCallbackFn;
};

class SFSClient
{
  public:
    ~SFSClient() noexcept;

    SFSClient(const SFSClient&) = delete;
    SFSClient& operator=(const SFSClient&) = delete;

    /**
     * @brief Make a new SFSClient object
     * @details An SFSClient object is used to make calls to the SFS service. The SFSClient object is initialized with
     * a few parameters that are used to build the URL for the SFS service. The URL is built as follows:
     * https://{accountId}.api.cdp.microsoft.com/api/v1/contents/{instanceId}/namespaces/{nameSpace}
     * The instanceId and nameSpace are optional and will default to a default value if not provided.
     *
     * @param options Set of Options that configure the SFSClient
     */
    [[nodiscard]] static Result Make(Options options, std::unique_ptr<SFSClient>& out) noexcept;

    //
    // API to retrieve download information from the SFS Service
    //

    /**
     * @brief Retrieve combined metadata & download URLs from the latest version of a specified product
     * @param productName The name of the product to retrieve
     * @param responseContents A vector of Content that is populated with the result
     */
    [[nodiscard]] Result GetLatestDownloadInfo(std::string_view productName, Contents& responseContents) const noexcept;

    /**
     * @brief Retrieve combined metadata & download URLs from the latest version of a specified product
     * @param productName The name of the product to retrieve
     * @param attributes Attributes to filter the search
     * @param responseContents A vector of Content that is populated with the result
     */
    [[nodiscard]] Result GetLatestDownloadInfo(std::string_view productName,
                                               const SearchAttributes& attributes,
                                               Contents& responseContents) const noexcept;

    //
    // API to retrieve optional extra download information from the SFS Service
    //

    /**
     * @brief Retrieve Delivery Optimization data for a given piece of content, if existing
     * @details If the Delivery Optimization data is not available, the result code will be set to E_NotSet
     * and the param data will not be modified
     * @param content A content object that was returned from a previous call to GetDownloadInfo
     * @param data A DeliveryOptimizationData object that is populated with the result
     */
    [[nodiscard]] Result GetDeliveryOptimizationData(const Content& content,
                                                     std::unique_ptr<DeliveryOptimizationData>& data) const noexcept;

    /**
     * @brief Retrieve Applicability details for a given piece of content, if existing
     * @details If the Applicability details are not available, the result code will be set to E_NotSet
     * and the param details will not be modified
     * @param content A content object that was returned from a previous call to GetDownloadInfo
     * @param data A ApplicabilityDetails object that is populated with the result
     */
    [[nodiscard]] Result GetApplicabilityDetails(const Content& content,
                                                 std::unique_ptr<ApplicabilityDetails>& details) const noexcept;

  private:
    /**
     * @brief Construct a new SFSClient object
     */
    SFSClient() noexcept;

    std::unique_ptr<details::SFSClientInterface> m_impl;
};
} // namespace SFS
