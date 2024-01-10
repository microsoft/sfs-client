#!/bin/bash

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Synopsis: Simplifies build commands for the SFS Client.
#
# Description: This script will contain the build commands for the SFS Client. The default build folder will be "<git_root>/build".
# Use this on non-Windows platforms in a bash session.
#
# Example:
# $ ./scripts/Build.sh
#

RED="\033[1;31m"
YELLOW="\033[1;33m"
NC="\033[0m" # No color

error() { echo -e "${RED}$*${NC}" >&2; }
warn() { echo -e "${YELLOW}$*${NC}"; }

clean=false

usage() { echo "Usage: $0 [--clean]" 1>&2; exit 1; }

if ! opts=$(getopt \
  --longoptions "clean" \
  --name "$(basename "$0")" \
  --options "" \
  -- "$@"
); then
    usage
fi

eval set "--$opts"

while [ $# -gt 0 ]; do
    case "$1" in
        --clean)
            clean=true
            shift 1
            ;;
        *)
            break
            ;;
    esac
done

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && pwd)"
git_root=$(git -C "$script_dir" rev-parse --show-toplevel)

vcpkg_dir="$git_root/vcpkg"
if [ ! -d "$vcpkg_dir" ]; then
    error "vcpkg not found at $git_root\vcpkg. Run the Setup.ps1 script first."
fi

build_folder="$git_root/build"
if $clean && [ -d "$build_folder" ]; then
    warn "Cleaning build folder before build..."
    rm -r -f "$build_folder"
fi

# Configure cmake if build folder doesn't exist. This creates build targets that will be used by the build command
if [ ! -d "$build_folder" ]; then
    cmake -S "$git_root" -B "$build_folder"
fi

# This is the build command. If any CMakeLists.txt files change, this will also reconfigure before building
cmake --build "$build_folder"
