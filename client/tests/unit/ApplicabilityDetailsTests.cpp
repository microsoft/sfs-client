// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/ApplicabilityDetails.h"

#include <gtest/gtest.h>

using namespace SFS;

TEST(ApplicabilityDetailsTests, Make)
{
    std::unique_ptr<ApplicabilityDetails> details;

    const std::vector<Architecture> architectures{Architecture::x86, Architecture::amd64};
    const std::vector<std::string> platformApplicabilityForPackage{"Windows.Desktop", "Windows.Server"};
    const std::string fileMoniker{"myApp"};

    ASSERT_EQ(
        ApplicabilityDetails::Make(architectures, platformApplicabilityForPackage, fileMoniker, details).GetCode(),
        Result::S_Ok);
    ASSERT_NE(nullptr, details);

    EXPECT_EQ(architectures, details->GetArchitectures());
    EXPECT_EQ(platformApplicabilityForPackage, details->GetPlatformApplicabilityForPackage());
    EXPECT_EQ(fileMoniker, details->GetFileMoniker());
}
