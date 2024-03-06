# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake")

# By default using x64 static custom triplet for Windows. Can be overridden by
# setting VCPKG_TARGET_TRIPLET
if(WIN32 AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-custom")
endif()

# Semver versioning. Given MAJOR.MINOR.PATCH, increment the:
#
# 1. MAJOR version when you make incompatible API changes
# 2. MINOR version when you add functionality in a backward compatible manner
# 3. PATCH version when you make backward compatible bug fixes
set(SFS_LIBRARY_VERSION "0.1.0")

project(
    SFSClient
    VERSION ${SFS_LIBRARY_VERSION}
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_SAMPLES "Indicates if samples should be built." OFF)

option(
    SFS_ENABLE_TEST_OVERRIDES
    "Set SFS_ENABLE_OVERRIDES to ON to enable certain test overrides through environment variables."
    OFF)

# Use this function to set warning level and warnings as errors for a given
# target
function(set_compile_options_for_target target)
    if(MSVC)
        # Enable some MSVC warnings that are off by default:
        #
        # * 4062: all enums are handled in switch statements without a default
        #   label
        # * 4191: unsafe conversion from 'type of expression' to 'type required'
        # * 4242: possible loss of data (identifier)
        # * 4254: possible loss of data (operator)
        # * 4287: 'operator': unsigned/negative constant mismatch
        # * 4296: 'operator': expression is always true (or false)
        # * 4388: signed/unsigned mismatch
        # * 4800: implicit conversion to bool; possible information loss
        # * 4946: reinterpret_cast used between related classes

        # cmake-format: off
        target_compile_options(${target} PRIVATE /W4 /WX /we4062 /we4191 /we4242 /we4254 /we4287 /we4296 /we4388 /we4800 /we4946)
        # cmake-format: on
    else()
        target_compile_options(
            ${target}
            PRIVATE -Wall
                    -Wextra
                    -Wpedantic
                    -Werror)
    endif()
endfunction()

# For MSVC, use a multi-threaded statically-linked runtime library
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(client)
add_subdirectory(tool)

if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()
