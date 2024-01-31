// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <nlohmann/json.hpp>

namespace SFS::details
{
class VersionResponse
{
  public:
    VersionResponse(nlohmann::json data) : m_responseData(data)
    {
    }

    // TODO: Placeholder for now

    const nlohmann::json& GetResponseData() const
    {
        return m_responseData;
    }

  private:
    nlohmann::json m_responseData;
};

class DownloadInfoResponse
{
  public:
    DownloadInfoResponse(nlohmann::json data) : m_responseData(data)
    {
    }

    // TODO: Placeholder for now

    const nlohmann::json& GetResponseData() const
    {
        return m_responseData;
    }

  private:
    nlohmann::json m_responseData;
};
} // namespace SFS::details
