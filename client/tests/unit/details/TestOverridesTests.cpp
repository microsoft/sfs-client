// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ReportingHandler.h"
#include "TestOverrides.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[TestOverridesTests] " __VA_ARGS__)

using namespace SFS::details;

TEST("Testing AreTestOverridesAllowed()")
{
    ReportingHandler handler;
    bool areTestOverridesAllowed = util::AreTestOverridesAllowed(handler);
#ifdef SFS_ENABLE_TEST_OVERRIDES
    REQUIRE(areTestOverridesAllowed);
#else
    REQUIRE_FALSE(areTestOverridesAllowed);
#endif
}

TEST("Testing GetEnv()")
{
    SECTION("Testing GetEnv() on an existing environment variable")
    {
        // Get the value of an existing environment variable per platform
#ifdef _WIN32
        const std::string varName = "COMPUTERNAME";
#else
        const std::string varName = "LANG";
#endif
        auto env = util::GetEnv(varName);
        REQUIRE(env.has_value());
        REQUIRE(env.value().size() > 0);
    }

    SECTION("Testing GetEnv() on a non-existing variable")
    {
        auto env = util::GetEnv("DUMMYVARIABLESHOULDNOTEXIST");
        REQUIRE(!env.has_value());
    }
}

TEST("Testing SetEnv()")
{
    SECTION("Testing SetEnv() on a non-existing variable")
    {
        auto env = util::GetEnv("DUMMYVARIABLE");
        REQUIRE(!env.has_value());

        REQUIRE(util::SetEnv("DUMMYVARIABLE", "dummyValue"));
        env = util::GetEnv("DUMMYVARIABLE");
        REQUIRE(env.has_value());
        REQUIRE(*env == "dummyValue");

        SECTION("Testing SetEnv() on an existing variable overwrites it")
        {
            REQUIRE(util::SetEnv("DUMMYVARIABLE", "dummyValue2"));

            env = util::GetEnv("DUMMYVARIABLE");
            REQUIRE(env.has_value());
            REQUIRE(*env == "dummyValue2");
        }

        INFO("Unsetting the environment variable");
        REQUIRE(util::UnsetEnv("DUMMYVARIABLE"));
        env = util::GetEnv("DUMMYVARIABLE");
        REQUIRE(!env.has_value());
    }

    SECTION("Testing SetEnv() with empty strings fails")
    {
        REQUIRE_FALSE(util::SetEnv("DUMMYVARIABLE", ""));
        REQUIRE_FALSE(util::SetEnv("DUMMYVARIABLE", std::string()));
        REQUIRE_FALSE(util::SetEnv("", "dummy"));
        REQUIRE_FALSE(util::SetEnv(std::string(), "dummy"));
        REQUIRE_FALSE(util::SetEnv(std::string(), std::string()));
    }
}

TEST("Testing UnsetEnv()")
{
    SECTION("Testing UnsetEnv() on a non-existing variable still succeeds")
    {
        auto env = util::GetEnv("DUMMYVARIABLE");
        REQUIRE(!env.has_value());

        REQUIRE(util::UnsetEnv("DUMMYVARIABLE"));
    }
}

TEST("Testing ScopedEnv")
{
    SECTION("Testing ScopedEnv on a non-existing variable")
    {
        auto env = util::GetEnv("DUMMYVARIABLE");
        REQUIRE(!env.has_value());

        {
            util::ScopedEnv scopedEnv("DUMMYVARIABLE", "dummyValue");

            INFO("Variable exists within scope");
            env = util::GetEnv("DUMMYVARIABLE");
            REQUIRE(env.has_value());
            REQUIRE(*env == "dummyValue");

            SECTION("Testing ScopedEnv on an existing variable overwrites it")
            {
                {
                    util::ScopedEnv scopedEnv2("DUMMYVARIABLE", "dummyValue2");

                    INFO("Variable has value overwritten within scope");
                    env = util::GetEnv("DUMMYVARIABLE");
                    REQUIRE(env.has_value());
                    REQUIRE(*env == "dummyValue2");
                }

                INFO("Variable goes back to previous value");
                env = util::GetEnv("DUMMYVARIABLE");
                REQUIRE(env.has_value());
                REQUIRE(*env == "dummyValue");
            }
        }

        INFO("Variable should be unset after the scope ends");
        env = util::GetEnv("DUMMYVARIABLE");
        REQUIRE(!env.has_value());
    }
}

TEST("Testing GetEnvironmentVariableFromOverride")
{
    REQUIRE(util::GetEnvironmentVariableFromOverride(util::TestOverride::BaseUrl) == "SFS_TEST_OVERRIDE_BASE_URL");
}

TEST("Testing GetTestOverride()")
{
    ReportingHandler handler;
    if (util::AreTestOverridesAllowed(handler))
    {
        SECTION("Testing GetTestOverride() on a non-existing environment variable")
        {
            auto env = util::GetTestOverride(util::TestOverride::BaseUrl, handler);
            REQUIRE(!env.has_value());

            SECTION("Testing GetTestOverride() on an existing environment variable")
            {
                const std::string varName = util::GetEnvironmentVariableFromOverride(util::TestOverride::BaseUrl);
                REQUIRE(util::SetEnv(varName, "override"));

                env = util::GetTestOverride(util::TestOverride::BaseUrl, handler);
                REQUIRE(env.has_value());
                REQUIRE(*env == "override");

                INFO("Unsetting the environment variable");
                REQUIRE(util::UnsetEnv(varName));
            }
        }
    }
    else
    {
        SECTION("GetTestOverride() returns std::nullopt when test overrides are not allowed")
        {
            auto env = util::GetTestOverride(util::TestOverride::BaseUrl, handler);
            REQUIRE(!env.has_value());
        }
    }
}

TEST("Testing ScopedTestOverride")
{
    ReportingHandler handler;
    SECTION("Testing ScopedTestOverride on a non-existing override")
    {
        const std::string varName = util::GetEnvironmentVariableFromOverride(util::TestOverride::BaseUrl);
        auto env = util::GetTestOverride(util::TestOverride::BaseUrl, handler);
        REQUIRE(!env.has_value());

        {
            util::ScopedTestOverride scopedOverride(util::TestOverride::BaseUrl, "dummyValue");

            INFO("Variable exists within scope");
            env = util::GetEnv(varName);
            REQUIRE(env.has_value());
            REQUIRE(*env == "dummyValue");

            if (util::AreTestOverridesAllowed(handler))
            {
                INFO("Checking test override within scope");
                env = util::GetTestOverride(util::TestOverride::BaseUrl, handler);
                REQUIRE(env.has_value());
                REQUIRE(*env == "dummyValue");
            }

            SECTION("Testing ScopedEnv on an existing variable overwrites it")
            {
                {
                    util::ScopedTestOverride scopedOverride2(util::TestOverride::BaseUrl, "dummyValue2");

                    INFO("Variable has value overwritten within scope");
                    env = util::GetEnv(varName);
                    REQUIRE(env.has_value());
                    REQUIRE(*env == "dummyValue2");

                    if (util::AreTestOverridesAllowed(handler))
                    {
                        INFO("Checking test override has been overwritten within scope");
                        env = util::GetTestOverride(util::TestOverride::BaseUrl, handler);
                        REQUIRE(env.has_value());
                        REQUIRE(*env == "dummyValue2");
                    }
                }

                INFO("Variable goes back to previous value");
                env = util::GetEnv(varName);
                REQUIRE(env.has_value());
                REQUIRE(*env == "dummyValue");

                if (util::AreTestOverridesAllowed(handler))
                {
                    INFO("Checking test override has gone back to previous value");
                    env = util::GetTestOverride(util::TestOverride::BaseUrl, handler);
                    REQUIRE(env.has_value());
                    REQUIRE(*env == "dummyValue");
                }
            }
        }

        INFO("Variable should be unset after the scope ends");
        env = util::GetEnv(varName);
        REQUIRE(!env.has_value());

        if (util::AreTestOverridesAllowed(handler))
        {
            INFO("Test override should be unset after the scope ends");
            env = util::GetTestOverride(util::TestOverride::BaseUrl, handler);
            REQUIRE(!env.has_value());
        }
    }
}
