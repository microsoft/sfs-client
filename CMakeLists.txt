# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake")

# By default using x64 static custom triplet for Windows. Can be overridden by
# setting VCPKG_TARGET_TRIPLET
if(WIN32 AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-custom")
endif()

project(
    SFSClient
    VERSION 0.1
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Setting warning level and warnings as errors
if(MSVC)
    # C4062: Checks all enum values are handled in switch statements without a
    # default label
    add_compile_options(/W4 /WX /we4062)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror)
endif()

# For MSVC, use a multi-threaded statically-linked runtime library
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(client)
add_subdirectory(tool)
