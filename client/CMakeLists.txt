# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# HTTP stack library
find_package(CURL REQUIRED)

# JSON library
find_package(nlohmann_json CONFIG REQUIRED)

# Include the capability of fetching external content
include(FetchContent)

# CorrelationVector Library from Microsoft
FetchContent_Declare(
    Microsoft.CorrelationVector
    GIT_REPOSITORY https://github.com/microsoft/CorrelationVector-Cpp.git
    GIT_TAG cf38d2b44baaf352509ad9980786bc49554c32e4 # last commit in 2019
)
FetchContent_MakeAvailable(Microsoft.CorrelationVector)

add_library(${PROJECT_NAME} STATIC)
add_library(Microsoft::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    PRIVATE src/ApplicabilityDetails.cpp
            src/Content.cpp
            src/DeliveryOptimizationData.cpp
            src/details/connection/Connection.cpp
            src/details/connection/ConnectionManager.cpp
            src/details/connection/CurlConnection.cpp
            src/details/connection/CurlConnectionManager.cpp
            src/details/connection/HttpHeader.cpp
            src/details/connection/mock/MockConnection.cpp
            src/details/connection/mock/MockConnectionManager.cpp
            src/details/ContentUtil.cpp
            src/details/CorrelationVector.cpp
            src/details/Env.cpp
            src/details/ErrorHandling.cpp
            src/details/ReportingHandler.cpp
            src/details/SFSClientImpl.cpp
            src/details/SFSException.cpp
            src/details/SFSUrlComponents.cpp
            src/details/TestOverride.cpp
            src/details/Util.cpp
            src/Logging.cpp
            src/Result.cpp
            src/SFSClient.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include/sfsclient)

target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE correlation_vector)

if(SFS_ENABLE_TEST_OVERRIDES)
    target_compile_definitions(${PROJECT_NAME}
                               PRIVATE SFS_ENABLE_TEST_OVERRIDES=1)
endif()

# Enables one to run tests through "ctest --test-dir .\build\client"
enable_testing()

add_subdirectory(tests)
