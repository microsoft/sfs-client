// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSClient.h"

#include "details/ErrorHandling.h"
#include "details/SFSClientImpl.h"
#include "details/connection/CurlConnectionManager.h"

using namespace SFS;
using namespace SFS::details;

// Defining the constructor and destructor here allows us to use a unique_ptr to SFSClientImpl in the header file
SFSClient::SFSClient() noexcept = default;
SFSClient::~SFSClient() noexcept = default;

Result SFSClient::Make(std::string accountId, std::unique_ptr<SFSClient>& out) noexcept
{
    return Make(std::move(accountId), {}, {}, out);
}

Result SFSClient::Make(std::string accountId, std::string instanceId, std::unique_ptr<SFSClient>& out) noexcept
{
    return Make(std::move(accountId), std::move(instanceId), {}, out);
}

Result SFSClient::Make(std::string accountId,
                       std::string instanceId,
                       std::string nameSpace,
                       std::unique_ptr<SFSClient>& out) noexcept
try
{
    out.reset();
    std::unique_ptr<SFSClient> tmp(new SFSClient());
    tmp->m_impl = std::make_unique<SFSClientImpl<CurlConnectionManager>>(std::move(accountId),
                                                                         std::move(instanceId),
                                                                         std::move(nameSpace));
    out = std::move(tmp);
    return Result::S_Ok;
}
SFS_CATCH_RETURN()

Result SFSClient::GetLatestDownloadInfo([[maybe_unused]] std::string_view productName,
                                        [[maybe_unused]] Contents& responseContents) const noexcept
try
{
    // return m_impl->GetDownloadInfo(...);
    return Result::E_NotImpl;
}
SFS_CATCH_RETURN()

Result SFSClient::GetLatestDownloadInfo([[maybe_unused]] std::string_view productName,
                                        [[maybe_unused]] const SearchAttributes& attributes,
                                        [[maybe_unused]] Contents& responseContents) const noexcept
try
{
    // return m_impl->GetDownloadInfo(...);
    return Result::E_NotImpl;
}
SFS_CATCH_RETURN()

Result SFSClient::GetDeliveryOptimizationData(
    [[maybe_unused]] const Content& content,
    [[maybe_unused]] std::unique_ptr<DeliveryOptimizationData>& data) const noexcept
try
{
    // return m_impl->GetDeliveryOptimizationData(...);
    return Result::E_NotImpl;
}
SFS_CATCH_RETURN()

Result SFSClient::GetApplicabilityDetails(
    [[maybe_unused]] const Content& content,
    [[maybe_unused]] std::unique_ptr<ApplicabilityDetails>& details) const noexcept
try
{
    // return m_impl->GetApplicabilityDetails(...);
    return Result::E_NotImpl;
}
SFS_CATCH_RETURN()

Result SFSClient::SetLoggingCallback(LoggingCallbackFn callback) noexcept
try
{
    m_impl->SetLoggingCallback(std::move(callback));
    return Result::S_Ok;
}
SFS_CATCH_RETURN()
