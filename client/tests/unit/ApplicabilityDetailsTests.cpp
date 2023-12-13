// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <sfsclient/ApplicabilityDetails.h>

#include <gtest/gtest.h>

using SFS::ApplicabilityDetails;
using SFS::Architecture;
using SFS::Result;

TEST(ApplicabilityDetailsTests, Make)
{
    std::unique_ptr<ApplicabilityDetails> details;

    std::vector<Architecture> architectures{Architecture::x86, Architecture::amd64};
    std::vector<std::string> platformApplicabilityForPackage{"Windows.Desktop", "Windows.Server"};
    std::string fileMoniker{"myApp"};

    ASSERT_TRUE(
        ApplicabilityDetails::Make(architectures, platformApplicabilityForPackage, fileMoniker, details).IsSuccess());
    ASSERT_NE(nullptr, details);

    EXPECT_EQ(architectures, details->GetArchitectures());
    EXPECT_EQ(platformApplicabilityForPackage, details->GetPlatformApplicabilityForPackage());
    EXPECT_EQ(fileMoniker, details->GetFileMoniker());
}
