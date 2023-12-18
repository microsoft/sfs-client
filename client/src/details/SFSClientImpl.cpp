// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "SFSClientImpl.h"

#include "Logging.h"
#include "Responses.h"

using namespace SFS;
using namespace SFS::details;

constexpr const char* c_apiDomain = "api.cdp.microsoft.com";

SFSClientImpl::SFSClientImpl(std::string&& accountId, std::string&& instanceId, std::string&& nameSpace)
    : m_accountId(accountId)
    , m_instanceId(instanceId)
    , m_nameSpace(nameSpace)
{
}

Result SFSClientImpl::GetLatestVersion([[maybe_unused]] std::string_view productName,
                                       [[maybe_unused]] const std::optional<SearchAttributes>& attributes,
                                       [[maybe_unused]] std::unique_ptr<VersionResponse>& response) const
{
    LOG_INFO(m_reportingHandler, "GetLatestVersion not implemented");
    return Result::E_NotImpl;
}

Result SFSClientImpl::GetSpecificVersion([[maybe_unused]] std::string_view productName,
                                         [[maybe_unused]] std::string_view version,
                                         [[maybe_unused]] const std::optional<SearchAttributes>& attributes,
                                         [[maybe_unused]] std::unique_ptr<VersionResponse>& content) const
{
    return Result::E_NotImpl;
}

Result SFSClientImpl::GetDownloadInfo([[maybe_unused]] std::string_view productName,
                                      [[maybe_unused]] std::string_view version,
                                      [[maybe_unused]] const std::optional<SearchAttributes>& attributes,
                                      [[maybe_unused]] std::unique_ptr<DownloadInfoResponse>& content) const
{
    return Result::E_NotImpl;
}

void SFSClientImpl::SetLoggingCallback(LoggingCallbackFn&& callback)
{
    m_reportingHandler.SetLoggingCallback(std::move(callback));
}

void SFSClientImpl::SetCustomBaseUrl(std::string customBaseUrl)
{
    m_customBaseUrl = std::move(customBaseUrl);
}

std::string SFSClientImpl::GetBaseUrl() const
{
    if (m_customBaseUrl)
    {
        return *m_customBaseUrl;
    }
    return "https://" + m_accountId + "." + std::string(c_apiDomain);
}
