// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "TestOverride.h"

#include "ErrorHandling.h"

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

Result ScopedTestOverride::Make(TestOverride override,
                                const std::string& value,
                                std::unique_ptr<ScopedTestOverride>& out)
{
    auto tmp = std::unique_ptr<ScopedTestOverride>(new ScopedTestOverride());
    RETURN_IF_FAILED(details::env::ScopedEnv::Make(GetEnvVarNameFromOverride(override), value, tmp->m_scopedEnv));
    out = std::move(tmp);
    return Result::Success;
}
