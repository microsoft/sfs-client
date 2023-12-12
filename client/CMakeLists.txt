# Copyright (c) Microsoft Corporation. All rights reserved.

add_library(${PROJECT_NAME} STATIC)
add_library(Microsoft::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    PRIVATE src/ApplicabilityDetails.cpp
            src/Content.cpp
            src/DeliveryOptimizationData.cpp
            src/Result.cpp
            src/SFSClient.cpp
            src/details/SFSClientImpl.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include/sfsclient)
