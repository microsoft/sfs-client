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
