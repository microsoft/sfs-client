# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

option(SFS_BUILD_INTEGRATION_DO_CLIENT "Build the DO client integration sample" ON)

if(SFS_BUILD_INTEGRATION_DO_CLIENT)
    add_subdirectory(integration-do-client)
endif()

add_subdirectory(tool)
