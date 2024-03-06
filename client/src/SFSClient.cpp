// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSClient.h"

#include "details/ContentUtil.h"
#include "details/ErrorHandling.h"
#include "details/ReportingHandler.h"
#include "details/SFSClientImpl.h"
#include "details/connection/Connection.h"
#include "details/connection/CurlConnectionManager.h"

using namespace SFS;
using namespace SFS::details;
using namespace SFS::details::contentutil;

namespace
{
void ValidateRequestParams(const RequestParams& requestParams)
{
    THROW_CODE_IF(InvalidArg, requestParams.productRequests.empty(), "productRequests cannot be empty");

    // TODO #78: Add support for multiple product requests
    THROW_CODE_IF(NotImpl,
                  requestParams.productRequests.size() > 1,
                  "There cannot be more than 1 productRequest at the moment");

    for (const auto& [product, _] : requestParams.productRequests)
    {
        THROW_CODE_IF(InvalidArg, product.empty(), "product cannot be empty");
    }
}
} // namespace

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

    LOG_INFO(out->m_impl->GetReportingHandler(), "SFSClient instance created successfully. Version: %s", GetVersion());
#ifdef SFS_GIT_INFO
    LOG_INFO(out->m_impl->GetReportingHandler(), "Git info: %s", SFS_GIT_INFO);
#endif

    return Result::Success;
}
SFS_CATCH_RETURN()

Result SFSClient::GetLatestDownloadInfo(const RequestParams& requestParams,
                                        std::unique_ptr<Content>& content) const noexcept
try
{
    ValidateRequestParams(requestParams);

    // TODO #50: Adapt retrieval to storeapps flow with pre-requisites once that is implemented server-side

    ConnectionConfig connectionConfig;
    connectionConfig.maxRetries = requestParams.retryOnError ? c_maxRetries : 0;
    connectionConfig.baseCV = requestParams.baseCV;
    const auto connection = m_impl->MakeConnection(connectionConfig);

    auto versionEntity = m_impl->GetLatestVersion(requestParams.productRequests[0], *connection);
    auto contentId = GenericVersionEntityToContentId(std::move(*versionEntity), m_impl->GetReportingHandler());

    const auto& product = requestParams.productRequests[0].product;
    auto files = m_impl->GetDownloadInfo(product, contentId->GetVersion(), *connection);

    std::unique_ptr<Content> tmp;
    RETURN_IF_FAILED_LOG(Content::Make(std::move(contentId), std::move(files), tmp), m_impl->GetReportingHandler());

    content = std::move(tmp);

    return Result::Success;
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

const char* SFSClient::GetVersion() noexcept
{
    return SFS_VERSION;
}
