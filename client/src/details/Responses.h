// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

// Replace with chosen JSON library, either nlohmann or Parson
class Json
{
};

namespace SFS::details
{
class VersionResponse
{
  public:
    VersionResponse() = default;

    // Placeholder for now

    Json GetResponseData() const;

  private:
    Json m_responseData;
};

class DownloadInfoResponse
{
  public:
    DownloadInfoResponse() = default;

    // Placeholder for now

    Json GetResponseData() const;

  private:
    Json m_responseData;
};
} // namespace SFS::details
