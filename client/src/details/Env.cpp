// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Env.h"

#include <cstdlib>

using namespace SFS;
using namespace SFS::details;
using SFS::details::env::ScopedEnv;

std::optional<std::string> env::GetEnv(const std::string& varName)
{
    if (varName.empty())
    {
        return std::nullopt;
    }
#ifdef _WIN32
    size_t len;
    char* buf;
    if (_dupenv_s(&buf, &len, varName.c_str()) == 0 && buf != nullptr)
    {
        std::string result{buf};
        free(buf);
        return result;
    }
#else
    if (const char* envValue = std::getenv(varName.c_str()))
    {
        return std::string(envValue);
    }
#endif
    // Return std::nullopt if the environment variable is not set or in case of failure
    return std::nullopt;
}

bool env::SetEnv(const std::string& varName, const std::string& value)
{
    if (varName.empty() || value.empty())
    {
        return false;
    }
#ifdef _WIN32
    return _putenv_s(varName.c_str(), value.c_str()) == 0;
#else
    return setenv(varName.c_str(), value.c_str(), 1 /*overwrite*/) == 0;
#endif
}

bool env::UnsetEnv(const std::string& varName)
{
    if (varName.empty())
    {
        return false;
    }
#ifdef _WIN32
    // On Windows, setting the value to an empty string is equivalent to unsetting the variable
    return _putenv_s(varName.c_str(), "") == 0;
#else
    return unsetenv(varName.c_str()) == 0;
#endif
}

Result ScopedEnv::Make(std::string varName, const std::string& value, std::unique_ptr<ScopedEnv>& out)
{
    auto tmp = std::unique_ptr<ScopedEnv>(new ScopedEnv(std::move(varName)));
    tmp->m_oldValue = GetEnv(tmp->m_varName);
    if (!SetEnv(tmp->m_varName, value))
    {
        return Result(Result::Unexpected, "Failed to set environment variable");
    }
    out = std::move(tmp);
    return Result::Success;
}

ScopedEnv::ScopedEnv(std::string varName) : m_varName(std::move(varName))
{
}

ScopedEnv::~ScopedEnv()
{
    if (m_oldValue)
    {
        SetEnv(m_varName, *m_oldValue);
    }
    else
    {
        UnsetEnv(m_varName);
    }
}
