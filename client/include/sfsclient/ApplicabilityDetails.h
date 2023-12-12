// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "Result.h"

#include <memory>
#include <string>
#include <vector>

namespace SFS
{
enum class Architecture
{
    x86,
    amd64
};

class ApplicabilityDetails
{
  public:
    [[nodiscard]] static Result Make(std::vector<Architecture> architectures,
                                     std::vector<std::string> platformApplicabilityForPackage,
                                     std::string fileMoniker,
                                     std::unique_ptr<ApplicabilityDetails>& out) noexcept;

    ApplicabilityDetails(const ApplicabilityDetails&) = delete;
    ApplicabilityDetails& operator=(const ApplicabilityDetails&) = delete;

    const std::vector<Architecture>& GetArchitectures() const noexcept;
    const std::vector<std::string>& GetPlatformApplicabilityForPackage() const noexcept;
    const std::string& GetFileMoniker() const noexcept;

  private:
    ApplicabilityDetails() = default;

    std::vector<Architecture> m_architectures;
    std::vector<std::string> m_platformApplicabilityForPackage;
    std::string m_fileMoniker;
};
} // namespace SFS
