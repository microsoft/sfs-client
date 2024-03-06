# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

project(IntegrationDOClient LANGUAGES CXX)

# Settings for do-client library
set(DO_INCLUDE_SDK
    ON
    CACHE BOOL "" FORCE)
set(DO_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif()

FetchContent_Declare(
    do-client
    GIT_REPOSITORY https://github.com/microsoft/do-client.git
    GIT_TAG d71ade6f692dd8bc319ec3228c956517e9b29292 # v1.1.0
)
FetchContent_MakeAvailable(do-client)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE IntegrationDOClient.cpp)

set(SFS_CLIENT_LIB_NAME "Microsoft::${CMAKE_PROJECT_NAME}")
add_dependencies(${PROJECT_NAME} ${SFS_CLIENT_LIB_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SFS_CLIENT_LIB_NAME})

set(DO_SDK_LIB_NAME "Microsoft::deliveryoptimization")
add_dependencies(${PROJECT_NAME} ${DO_SDK_LIB_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DO_SDK_LIB_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ../../client/include)
