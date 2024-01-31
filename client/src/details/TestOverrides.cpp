// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "TestOverrides.h"

#include "ReportingHandler.h"

#include <cstdlib>
#include <string>

using namespace SFS::details;

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
    size_t len;
    char* buf;
    if (_dupenv_s(&buf, &len, varName) == 0 && buf != nullptr)
    {
        return std::string(buf);
    }
#else
    if (const char* envValue = std::getenv(varName))
    {
        return std::string(envValue);
    }
#endif
    // Return std::nullopt if the environment variable is not set or in case of failure
    return std::nullopt;
}

bool util::SetEnv(const char* varName, const char* value)
{
#ifdef WIN32
    return _putenv_s(varName, value) == 0;
#else
    return setenv(varName, value, 1) == 0;
#endif
}

bool util::UnsetEnv(const char* varName)
{
#ifdef WIN32
    // On Windows, setting the value to an empty string is equivalent to unsetting the variable
    return _putenv_s(varName, "") == 0;
#else
    return unsetenv(varName) == 0;
#endif
}

util::ScopedEnv::ScopedEnv(std::string varName, const char* value) : m_varName(std::move(varName))
{
    m_oldValue = GetEnv(m_varName.c_str());
    if (!SetEnv(m_varName.c_str(), value))
    {
        m_oldValue.reset();
    }
}

util::ScopedEnv::~ScopedEnv()
{
    if (m_oldValue)
    {
        SetEnv(m_varName.c_str(), m_oldValue->c_str());
    }
    else
    {
        UnsetEnv(m_varName.c_str());
    }
}
