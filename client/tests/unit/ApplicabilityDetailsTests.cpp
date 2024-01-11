// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/ApplicabilityDetails.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[ApplicabilityDetailsTests] " __VA_ARGS__)

using namespace SFS;

namespace
{
std::unique_ptr<ApplicabilityDetails> GetDetails(const std::vector<Architecture>& architectures,
                                                 const std::vector<std::string>& platformApplicabilityForPackage,
                                                 const std::string& fileMoniker)
{
    std::unique_ptr<ApplicabilityDetails> details;
    REQUIRE(ApplicabilityDetails::Make(architectures, platformApplicabilityForPackage, fileMoniker, details) ==
            Result::S_Ok);
    REQUIRE(details != nullptr);
    return details;
};
} // namespace

TEST("Testing ApplicabilityDetails::Make()")
{
    const std::vector<Architecture> architectures{Architecture::x86, Architecture::amd64};
    const std::vector<std::string> platformApplicabilityForPackage{"Windows.Desktop", "Windows.Server"};
    const std::string fileMoniker{"myApp"};

    const std::unique_ptr<ApplicabilityDetails> details =
        GetDetails(architectures, platformApplicabilityForPackage, fileMoniker);

    CHECK(architectures == details->GetArchitectures());
    CHECK(platformApplicabilityForPackage == details->GetPlatformApplicabilityForPackage());
    CHECK(fileMoniker == details->GetFileMoniker());

    SECTION("Testing ApplicabilityDetails equality operators")
    {
        SECTION("Equal")
        {
            auto sameDetails = GetDetails(architectures, platformApplicabilityForPackage, fileMoniker);
            REQUIRE(*details == *sameDetails);
            REQUIRE_FALSE(*details != *sameDetails);
        }

        SECTION("Not equal")
        {
            auto CompareDetailsNotEqual = [&details](const std::unique_ptr<ApplicabilityDetails>& otherDetails) {
                REQUIRE(*details != *otherDetails);
                REQUIRE_FALSE(*details == *otherDetails);
            };

            CompareDetailsNotEqual(GetDetails({}, platformApplicabilityForPackage, fileMoniker));
            CompareDetailsNotEqual(GetDetails(architectures, {}, fileMoniker));
            CompareDetailsNotEqual(GetDetails(architectures, platformApplicabilityForPackage, ""));
            CompareDetailsNotEqual(GetDetails({}, {}, ""));
        }
    }
}
