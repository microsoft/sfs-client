# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

option(SFS_BUILD_INTEGRATION_DO_CLIENT "Build the DO client integration sample"
       ON)
option(SFS_SKIP_DO_CLIENT_DOWNLOAD
       "Skip downloading DO client (for testing only)" OFF)

if(SFS_BUILD_INTEGRATION_DO_CLIENT)
    if(NOT SFS_SKIP_DO_CLIENT_DOWNLOAD)
        add_subdirectory(integration-do-client)
    else()
        message(
            STATUS "Skipping DO client integration sample download as requested"
        )
    endif()
endif()

add_subdirectory(tool)
