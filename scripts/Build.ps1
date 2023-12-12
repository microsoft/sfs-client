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
$BuildFolder = "$GitRoot/build"
$BuildTimestampsFile = "$GitRoot\build.timestamps"

function GetTimestamps {
    $Dict = @{}
    $CMakeFiles = (Get-ChildItem -Exclude vcpkg | Get-ChildItem -Recurse -Include "CMakeLists.txt").FullName
    foreach ($CMakeFile in $CMakeFiles)
    {
        $CMakeFileTimestamp = ([DateTimeOffset](Get-Item $CMakeFile).LastWriteTime).ToUnixTimeSeconds()
        $Dict.Add($CMakeFile, $CMakeFileTimestamp)
    }

    return $Dict
}

function CheckTimestamps {
    Param(
        [hashtable]$Timestamps
    )

    $LastSavedTimestamps = Get-Content $BuildTimestampsFile | ConvertFrom-Json
    foreach ($i in $Timestamps.GetEnumerator())
    {
        $CMakeFile = $i.Name
        $Timestamp = $i.Value

        if (!([bool]($LastSavedTimestamps.PSobject.Properties.name -match $CMakeFile.Replace("\", "\\"))))
        {
            Write-Host -ForegroundColor Yellow "$CMakeFile is a new file. Re-configuring CMake`n"
            return $true
        }

        if ($LastSavedTimestamps.$CMakeFile -ne $Timestamp)
        {
            Write-Host -ForegroundColor Yellow "$CMakeFile has changed. Re-configuring CMake`n"
            return $true
        }
    }

    return $false
}

function WriteTimestamps {
    Param(
        [hashtable]$Timestamps
    )

    $Timestamps | ConvertTo-Json | Out-File $BuildTimestampsFile
}

function ConfigureCMake {

    $Reconfigure = $false

    # If build does not exist yet, we need to configure
    if (!(Test-Path $BuildFolder) -or !(Test-Path $BuildTimestampsFile))
    {
        $Reconfigure = $true
    }

    # If a timestamp has changed, we need to configure
    $Timestamps = GetTimestamps
    if (!$Reconfigure)
    {
        $Reconfigure = CheckTimestamps -Timestamps $Timestamps
    }

    if ($Reconfigure)
    {
        cmake -S $GitRoot -B $GitRoot/build
        WriteTimestamps -Timestamps $Timestamps
    }
}

if (!(Test-Path "$GitRoot\vcpkg"))
{
    throw "vcpkg not found at $GitRoot\vcpkg. Run the Setup.ps1 script first."
}

if ($Clean)
{
    Write-Host -ForegroundColor Yellow "Cleaning build folder"
    Remove-Item -Recurse -Force $BuildFolder
    Remove-Item -Force $BuildTimestampsFile
}

ConfigureCMake

# Build command
cmake --build $BuildFolder
