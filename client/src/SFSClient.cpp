// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSClient.h"

#include "details/ErrorHandling.h"
#include "details/SFSClientImpl.h"
#include "details/connection/Connection.h"
#include "details/connection/CurlConnectionManager.h"

using namespace SFS;
using namespace SFS::details;

// Defining the constructor and destructor here allows us to use a unique_ptr to SFSClientImpl in the header file
SFSClient::SFSClient() noexcept = default;
SFSClient::~SFSClient() noexcept = default;

Result SFSClient::Make(ClientConfig config, std::unique_ptr<SFSClient>& out) noexcept
try
{
    if (config.accountId.empty())
    {
        return Result(Result::InvalidArg, "ClientConfig::accountId cannot be empty");
    }

    out.reset();
    std::unique_ptr<SFSClient> tmp(new SFSClient());
    tmp->m_impl = std::make_unique<details::SFSClientImpl<CurlConnectionManager>>(std::move(config));
    out = std::move(tmp);
    return Result::Success;
}
SFS_CATCH_RETURN()

Result SFSClient::GetLatestDownloadInfo([[maybe_unused]] const std::string& productName,
                                        [[maybe_unused]] std::unique_ptr<Content>& content) const noexcept
try
{
    return GetLatestDownloadInfo(productName, SearchAttributes(), content);
}
SFS_CATCH_RETURN()

Result SFSClient::GetLatestDownloadInfo([[maybe_unused]] const std::string& productName,
                                        [[maybe_unused]] const SearchAttributes& attributes,
                                        [[maybe_unused]] std::unique_ptr<Content>& content) const noexcept
try
{
    if (productName.empty())
    {
        return Result(Result::InvalidArg, "productName cannot be empty");
    }

    // TODO #50: Adapt retrieval to storeapps flow with pre-requisites once that is implemented server-side

    const auto connection = m_impl->MakeConnection();

    auto contentId = m_impl->GetLatestVersion(productName, attributes, *connection);
    auto files = m_impl->GetDownloadInfo(productName, contentId->GetVersion(), *connection);

    std::unique_ptr<Content> tmp;
    RETURN_IF_FAILED_LOG(Content::Make(std::move(contentId), std::move(files), tmp), m_impl->GetReportingHandler());

    content = std::move(tmp);

    return Result::Success;
}
SFS_CATCH_RETURN()

Result SFSClient::GetDeliveryOptimizationData(
    [[maybe_unused]] const Content& content,
    [[maybe_unused]] std::unique_ptr<DeliveryOptimizationData>& data) const noexcept
try
{
    // return m_impl->GetDeliveryOptimizationData(...);
    return Result::NotImpl;
}
SFS_CATCH_RETURN()

Result SFSClient::GetApplicabilityDetails(
    [[maybe_unused]] const Content& content,
    [[maybe_unused]] std::unique_ptr<ApplicabilityDetails>& details) const noexcept
try
{
    // return m_impl->GetApplicabilityDetails(...);
    return Result::NotImpl;
}
SFS_CATCH_RETURN()
