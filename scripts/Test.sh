#!/bin/bash

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Synopsis: Simplifies test commands for the SFS Client.
#
# Description: This script will contain the test commands for the SFS Client.
# Use this on non-Windows platforms in a bash session.
#
# Example:
# $ ./scripts/Test.sh
#

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && pwd)"
git_root=$(git -C "$script_dir" rev-parse --show-toplevel)
build_folder="$git_root/build"

ctest --test-dir "$build_folder/client"
