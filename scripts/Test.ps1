# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

<#
.SYNOPSIS
Simplifies test commands for the SFS Client.

.DESCRIPTION
This script will contain the test commands for the SFS Client.
Use this on Windows platforms in a PowerShell session.

.EXAMPLE
PS> ./scripts/Test.ps1
#>

$GitRoot = (Resolve-Path (&git -C $PSScriptRoot rev-parse --show-toplevel)).Path
$BuildFolder = "$GitRoot/build"

ctest --test-dir "$BuildFolder/client"
