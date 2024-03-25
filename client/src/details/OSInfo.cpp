// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "OSInfo.h"

#ifdef _WIN32

#include <windows.h>

#include <VersionHelpers.h>
#include <sysinfoapi.h>

#elif __GNUG__

#include <filesystem>
#include <fstream>
#include <sys/utsname.h>

#endif

#include <sstream>

using namespace SFS::details;

namespace
{
#ifdef _WIN32
std::string GetWindowsOSVersion()
{
    // In Windows, the API below only works if the application has a manifest file with indications to the supported OS
    // version (https://learn.microsoft.com/en-us/windows/win32/sysinfo/targeting-your-application-at-windows-8-1).
    // If the caller application does not have a manifest file, this function will just return "Windows"
    return IsWindows10OrGreater() ? "Windows NT 10.0" : "Windows";
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
    // Check for /etc/os-release file, which contains information about the distribution
    static const std::string s_osReleaseFile = "/etc/os-release";
    if (std::filesystem::exists(s_osReleaseFile))
    {
        std::ifstream file(s_osReleaseFile);
        std::string line;
        while (std::getline(file, line))
        {
            // The PRETTY_NAME line contains the name of the distribution like PRETTY_NAME="Ubuntu 22.04.3 LTS"
            if (line.find("PRETTY_NAME") != std::string::npos)
            {
                // Extract the name of the distribution by removing the quotes
                return line.substr(line.find("\"") + 1, line.rfind("\"") - line.find("\"") - 1);
            }
        }
    }

    // Linux distro not found, return kernel name
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
