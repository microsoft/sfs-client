# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

enable_testing()

find_package(GTest CONFIG REQUIRED)

set(SFS_CLIENT_TESTS "SFSClientTests")

add_executable(${SFS_CLIENT_TESTS})

target_sources(${SFS_CLIENT_TESTS} PRIVATE unit/ApplicabilityDetailsTests.cpp
                                           unit/ContentTests.cpp main.cpp)

set(SFS_CLIENT_LIB_NAME "Microsoft::${PROJECT_NAME}")
add_dependencies(${SFS_CLIENT_TESTS} ${SFS_CLIENT_LIB_NAME})
target_link_libraries(
    ${SFS_CLIENT_TESTS}
    PRIVATE ${SFS_CLIENT_LIB_NAME}
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
            GTest::gmock_main)

target_include_directories(${SFS_CLIENT_TESTS} PUBLIC ../include)

add_test(Client ${SFS_CLIENT_TESTS})

set_target_properties(
    ${SFS_CLIENT_TESTS} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                   ${CMAKE_BINARY_DIR}/tests/bin/)
