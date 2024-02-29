// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSClient.h"

#include "details/ErrorHandling.h"
#include "details/ReportingHandler.h"
#include "details/SFSClientImpl.h"
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

    LOG_INFO(out->m_impl->GetReportingHandler(), "SFSClient instance created successfully. Version: %s", GetVersion());

    return Result::Success;
}
SFS_CATCH_RETURN()

Result SFSClient::GetLatestDownloadInfo(const RequestParams& requestParams,
                                        std::unique_ptr<Content>& content) const noexcept
try
{
    content = m_impl->GetLatestDownloadInfo(requestParams);
    return Result::Success;
}
SFS_CATCH_RETURN()

Result SFSClient::GetLatestAppDownloadInfo(const RequestParams& requestParams,
                                           std::unique_ptr<AppContent>& content) const noexcept
try
{
    content = m_impl->GetLatestAppDownloadInfo(requestParams);
    return Result::Success;
}
SFS_CATCH_RETURN()

const char* SFSClient::GetVersion() noexcept
{
#ifdef SFS_GIT_INFO
    return SFS_VERSION " (" SFS_GIT_INFO ")";
#else
    return SFS_VERSION;
#endif
}
