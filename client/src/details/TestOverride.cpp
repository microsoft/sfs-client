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
    case TestOverride::NoConnectionConfigLimits:
        return "SFS_TEST_OVERRIDE_NO_CONNECTION_CONFIG_LIMITS";
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

bool test::HasTestOverride(TestOverride override)
{
    return GetTestOverride(override).has_value();
}

ScopedTestOverride::ScopedTestOverride(TestOverride override, const std::string& value)
    : m_scopedEnv(GetEnvVarNameFromOverride(override), value)
{
}
