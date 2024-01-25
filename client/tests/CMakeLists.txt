# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

find_package(Catch2 CONFIG REQUIRED)
include(Catch)

# For mock http server
find_package(httplib CONFIG REQUIRED)

project(SFSClientTests LANGUAGES CXX)

add_executable(${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    PRIVATE mock/MockWebServer.cpp
            unit/ApplicabilityDetailsTests.cpp
            unit/ContentTests.cpp
            unit/DeliveryOptimizationDataTests.cpp
            unit/details/ErrorHandlingTests.cpp
            unit/details/CurlConnectionManagerTests.cpp
            unit/details/ReportingHandlerTests.cpp
            unit/details/SFSClientImplTests.cpp
            unit/ResultTests.cpp
            unit/SFSClientTests.cpp)

set(SFS_CLIENT_LIB_NAME "Microsoft::${CMAKE_PROJECT_NAME}")
add_dependencies(${PROJECT_NAME} ${SFS_CLIENT_LIB_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SFS_CLIENT_LIB_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${PROJECT_NAME} PRIVATE httplib::httplib)

target_include_directories(${PROJECT_NAME} PUBLIC ../include)
target_include_directories(${PROJECT_NAME} PUBLIC ../src/details)

catch_discover_tests(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                 ${CMAKE_BINARY_DIR}/tests/bin/)
