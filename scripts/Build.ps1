# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

<#
.SYNOPSIS
Simplifies build commands for the SFS Client.

.DESCRIPTION
This script will contain the build commands for the SFS Client. The default build folder will be "<git_root>/build".
Use this on Windows platforms in a PowerShell session.

.EXAMPLE
PS> ./scripts/Build.ps1
#>
param (
    [switch] $Clean
)

$GitRoot = (Resolve-Path (&git -C $PSScriptRoot rev-parse --show-toplevel)).Path
if (!(Test-Path "$GitRoot\vcpkg"))
{
    throw "vcpkg not found at $GitRoot\vcpkg. Run the Setup.ps1 script first."
}

$BuildFolder = "$GitRoot/build"
if ($Clean -and (Test-Path $BuildFolder))
{
    Write-Host -ForegroundColor Yellow "Cleaning build folder before build..."
    Remove-Item -Recurse -Force $BuildFolder
}

# Configure cmake if build folder doesn't exist. This creates build targets that will be used by the build command
if (!(Test-Path $BuildFolder))
{
    cmake -S $GitRoot -B $BuildFolder
}

# This is the build command. If any CMakeLists.txt files change, this will also reconfigure before building
cmake --build $BuildFolder
