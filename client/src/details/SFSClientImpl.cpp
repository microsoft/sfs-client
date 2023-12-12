// Copyright (c) Microsoft Corporation. All rights reserved.

#include "SFSClientImpl.h"

#include "Responses.h"

using namespace SFS;
using namespace SFS::details;

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
