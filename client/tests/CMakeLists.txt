# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

find_package(Catch2 CONFIG REQUIRED)
include(Catch)

set(SFS_CLIENT_TESTS "SFSClientTests")

add_executable(${SFS_CLIENT_TESTS})

target_sources(
    ${SFS_CLIENT_TESTS}
    PRIVATE unit/ApplicabilityDetailsTests.cpp
            unit/ContentTests.cpp
            unit/DeliveryOptimizationDataTests.cpp
            unit/details/ErrorHandlingTests.cpp
            unit/details/ReportingHandlerTests.cpp
            unit/details/SFSClientImplTests.cpp
            unit/ResultTests.cpp
            unit/SFSClientTests.cpp)

set(SFS_CLIENT_LIB_NAME "Microsoft::${PROJECT_NAME}")
add_dependencies(${SFS_CLIENT_TESTS} ${SFS_CLIENT_LIB_NAME})
target_link_libraries(${SFS_CLIENT_TESTS} PRIVATE ${SFS_CLIENT_LIB_NAME})
target_link_libraries(${SFS_CLIENT_TESTS} PRIVATE Catch2::Catch2WithMain)

target_include_directories(${SFS_CLIENT_TESTS} PUBLIC ../include)
target_include_directories(${SFS_CLIENT_TESTS} PUBLIC ../src/details)

catch_discover_tests(${SFS_CLIENT_TESTS})

set_target_properties(
    ${SFS_CLIENT_TESTS} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                   ${CMAKE_BINARY_DIR}/tests/bin/)
