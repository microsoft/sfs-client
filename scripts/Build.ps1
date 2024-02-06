# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

<#
.SYNOPSIS
Simplifies build commands for the SFS Client.

.PARAMETER Clean
Use this to clean the build folder before building.

.PARAMETER EnableTestOverrides
Use this to enable test overrides.

.DESCRIPTION
This script will contain the build commands for the SFS Client. The default build folder will be "<git_root>/build".
Use this on Windows platforms in a PowerShell session.

.EXAMPLE
PS> ./scripts/Build.ps1
#>
param (
    [switch] $Clean = $false,
    [switch] $EnableTestOverrides = $false
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

$Regenerate = $false
$CMakeCacheFile = "$BuildFolder\CMakeCache.txt"
$EnableTestOverridesStr = if ($EnableTestOverrides) {"ON"} else {"OFF"}

if (Test-Path $CMakeCacheFile)
{
    # Regenerate if the SFS_ENABLE_TEST_OVERRIDES option is set to a different value than the one passed in
    $MatchTestOverrides = Select-String -Path $CMakeCacheFile -Pattern "^SFS_ENABLE_TEST_OVERRIDES:BOOL=(.*)$"
    if ($null -ne $MatchTestOverrides -and $null -ne $MatchTestOverrides.Matches.Groups[1])
    {
        $Regenerate = $EnableTestOverridesStr -ne $MatchTestOverrides.Matches.Groups[1].Value
    }
}

# Configure cmake if build folder doesn't exist or if the build must be regenerated.
# This creates build targets that will be used by the build command
if (!(Test-Path $BuildFolder) -or $Regenerate)
{
    cmake -S $GitRoot -B $BuildFolder "-DSFS_ENABLE_TEST_OVERRIDES=$EnableTestOverridesStr"
}

# This is the build command. If any CMakeLists.txt files change, this will also reconfigure before building
cmake --build $BuildFolder
