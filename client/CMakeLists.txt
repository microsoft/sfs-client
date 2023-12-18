# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# HTTP stack library
find_package(CURL REQUIRED)

add_library(${PROJECT_NAME} STATIC)
add_library(Microsoft::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    PRIVATE src/ApplicabilityDetails.cpp
            src/Content.cpp
            src/DeliveryOptimizationData.cpp
            src/Logging.cpp
            src/Result.cpp
            src/SFSClient.cpp
            src/details/ReportingHandler.cpp
            src/details/SFSClientImpl.cpp
            src/details/Util.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include/sfsclient)

target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)

# Enables one to run tests through "ctest --test-dir .\build\client"
enable_testing()

add_subdirectory(tests)
