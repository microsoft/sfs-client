# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

<#
.SYNOPSIS
Simplifies build commands for the SFS Client.

.PARAMETER Clean
Use this to clean the build folder before building.

.PARAMETER EnableTestOverrides
Use this to enable test overrides. For this switch to work correctly, either the CMake build folder must have never been generated, or either the Clean or the Regenerate switch must be used to force the reconfiguration of the build cache.

.PARAMETER Regenerate
Use this to regenerate the CMake build targets.

.DESCRIPTION
This script will contain the build commands for the SFS Client. The default build folder will be "<git_root>/build".
Use this on Windows platforms in a PowerShell session.

.EXAMPLE
PS> ./scripts/Build.ps1
#>
param (
    [switch] $Clean = $false,
    [switch] $EnableTestOverrides = $false,
    [switch] $Regenerate = $false
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

if ($EnableTestOverrides -and !$Regenerate -and !$Clean -and (Test-Path $BuildFolder))
{
    Write-Warning "For the EnableTestOverrides switch to work if the CMake build has already been generated, either -Clean or -Regenerate must be passed to force the reconfiguration of the build cache. Otherwise, the switch will have no effect and the value used during the last configuration will be preserved."
}

# Configure cmake if build folder doesn't exist or if Regenerate switch is used.
# This creates build targets that will be used by the build command
if (!(Test-Path $BuildFolder) -or $Regenerate)
{
    $EnableTestOverridesStr = if ($EnableTestOverrides) {"ON"} else {"OFF"}
    cmake -S $GitRoot -B $BuildFolder "-DSFS_ENABLE_TEST_OVERRIDES=$EnableTestOverridesStr"
}

# This is the build command. If any CMakeLists.txt files change, this will also reconfigure before building
cmake --build $BuildFolder
