// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "OSInfo.h"

#ifdef _WIN32

#include <windows.h>

#include <sysinfoapi.h>

#elif __GNUG__

#include <sys/utsname.h>

#endif

using namespace SFS::details;

namespace
{
#ifdef _WIN32
std::string GetWindowsOSVersion()
{
    return "Windows";
}

std::string GetWindowsMachineInfo()
{
    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo(&systemInfo);

    switch (systemInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        return "x64";
    case PROCESSOR_ARCHITECTURE_ARM:
        return "ARM";
    case PROCESSOR_ARCHITECTURE_ARM64:
        return "ARM64";
    case PROCESSOR_ARCHITECTURE_INTEL:
        return "x86";
    default:
        return "Unknown";
    }
}
#endif

#ifdef __GNUG__
std::string GetLinuxOSVersion()
{
    // Return kernel name, usually "Linux"
    utsname buf;
    if (uname(&buf) >= 0)
    {
        return buf.sysname;
    }
    return "Unknown";
}

std::string GetLinuxMachineInfo()
{
    utsname buf;
    if (uname(&buf) >= 0)
    {
        return buf.machine;
    }
    return "Unknown";
}
#endif
} // namespace

std::string osinfo::GetOSVersion()
{
#ifdef _WIN32
    return GetWindowsOSVersion();
#elif __GNUG__
    return GetLinuxOSVersion();
#else
    return "Unknown OS";
#endif
}

std::string osinfo::GetOSMachineInfo()
{
#ifdef _WIN32
    return GetWindowsMachineInfo();
#elif __GNUG__
    return GetLinuxMachineInfo();
#else
    return "Unknown Machine";
#endif
}
