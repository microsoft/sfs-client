# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

set(SFS_CLIENT_TOOL_NAME "SFSClientTool")

add_executable(${SFS_CLIENT_TOOL_NAME})

target_sources(${SFS_CLIENT_TOOL_NAME} PRIVATE SFSClientTool.cpp)

set(SFS_CLIENT_LIB_NAME "Microsoft::${PROJECT_NAME}")
add_dependencies(${SFS_CLIENT_TOOL_NAME} ${SFS_CLIENT_LIB_NAME})
target_link_libraries(${SFS_CLIENT_TOOL_NAME} ${SFS_CLIENT_LIB_NAME})

target_include_directories(${SFS_CLIENT_TOOL_NAME} PUBLIC ../client/include)
