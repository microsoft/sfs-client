// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "TestOverride.h"

using namespace SFS;
using SFS::test::ScopedTestOverride;
using SFS::test::TestOverride;

bool test::AreTestOverridesAllowed()
{
#ifdef SFS_ENABLE_TEST_OVERRIDES
    return true;
#else
    return false;
#endif
}

std::string test::GetEnvVarNameFromOverride(TestOverride override)
{
    switch (override)
    {
    case TestOverride::BaseUrl:
        return "SFS_TEST_OVERRIDE_BASE_URL";
    case TestOverride::PublicKey:
        return "SFS_TEST_OVERRIDE_PUBLIC_KEY";
    }
    return "";
}

std::optional<std::string> test::GetTestOverride(TestOverride override)
{
    if (!AreTestOverridesAllowed())
    {
        return std::nullopt;
    }

    return details::env::GetEnv(GetEnvVarNameFromOverride(override));
}

ScopedTestOverride::ScopedTestOverride(TestOverride override, const std::string& value)
    : m_scopedEnv(GetEnvVarNameFromOverride(override), value)
{
}
