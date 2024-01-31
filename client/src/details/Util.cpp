// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Util.h"

#include "ReportingHandler.h"

#include <cstdlib>
#include <string>

using namespace SFS::details;

bool util::AreEqualI(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
    {
        return false;
    }

    for (auto itA = a.begin(), itB = b.begin(); itA != a.end() && itB != b.end(); ++itA, ++itB)
    {
        if (std::tolower(*itA) != std::tolower(*itB))
        {
            return false;
        }
    }
    return true;
}

bool util::AreNotEqualI(std::string_view a, std::string_view b)
{
    return !AreEqualI(a, b);
}

bool util::AreTestOverridesAllowed(const ReportingHandler& handler)
{
#ifdef SFS_ENABLE_TEST_OVERRIDES
    LOG_INFO(handler, "Test overrides are enabled");
    return true;
#else
    return false;
#endif
}

std::optional<std::string> util::GetEnv(const char* varName)
{
#ifdef WIN32
    static constexpr size_t s_maxEnvValueLength = 200;
    size_t len = 0;
    char buf[s_maxEnvValueLength];
    if (getenv_s(&len, buf, sizeof(buf), varName) == 0)
    {
        return std::string(buf);
    }
#else
    if (const char* envValue = std::getenv(varName))
    {
        return std::string(envValue);
    }
#endif
    return std::nullopt;
}
