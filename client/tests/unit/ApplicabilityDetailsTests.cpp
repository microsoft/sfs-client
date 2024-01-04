// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/ApplicabilityDetails.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[ApplicabilityDetailsTests] " __VA_ARGS__)

using namespace SFS;

TEST("Testing ApplicabilityDetails::Make()")
{
    std::unique_ptr<ApplicabilityDetails> details;

    const std::vector<Architecture> architectures{Architecture::x86, Architecture::amd64};
    const std::vector<std::string> platformApplicabilityForPackage{"Windows.Desktop", "Windows.Server"};
    const std::string fileMoniker{"myApp"};

    REQUIRE(ApplicabilityDetails::Make(architectures, platformApplicabilityForPackage, fileMoniker, details) ==
            Result::S_Ok);
    REQUIRE(details != nullptr);

    CHECK(architectures == details->GetArchitectures());
    CHECK(platformApplicabilityForPackage == details->GetPlatformApplicabilityForPackage());
    CHECK(fileMoniker == details->GetFileMoniker());
}
