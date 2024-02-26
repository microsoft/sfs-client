// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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
                                     std::unique_ptr<ApplicabilityDetails>& out) noexcept;

    ApplicabilityDetails(const ApplicabilityDetails&) = delete;
    ApplicabilityDetails& operator=(const ApplicabilityDetails&) = delete;

    const std::vector<Architecture>& GetArchitectures() const noexcept;
    const std::vector<std::string>& GetPlatformApplicabilityForPackage() const noexcept;

    bool operator==(const ApplicabilityDetails& other) const noexcept;
    bool operator!=(const ApplicabilityDetails& other) const noexcept;

  private:
    ApplicabilityDetails() = default;

    std::vector<Architecture> m_architectures;
    std::vector<std::string> m_platformApplicabilityForPackage;
};
} // namespace SFS
