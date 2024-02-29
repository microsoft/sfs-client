# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

project(SFSClientTool LANGUAGES CXX)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE SFSClientTool.cpp)

set(SFS_CLIENT_LIB_NAME "Microsoft::${CMAKE_PROJECT_NAME}")
add_dependencies(${PROJECT_NAME} ${SFS_CLIENT_LIB_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE ${SFS_CLIENT_LIB_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ../client/include)
