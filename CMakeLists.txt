# Copyright (c) Microsoft Corporation. All rights reserved.

cmake_minimum_required(VERSION 3.13)

project(
    SFSClient
    VERSION 0.1
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Setting warning level and warnings as errors
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror)
endif()

add_subdirectory(client)
add_subdirectory(tool)
