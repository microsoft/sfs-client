// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdint>
#include <regex>
#include <string>

namespace SFS::sample
{
inline bool IsValidFileId(const std::string& fileId)
{
    static const std::regex c_validFileId("^[a-zA-Z0-9._-]+$");
    return std::regex_match(fileId, c_validFileId);
}

// Encode each byte as two hex digits to avoid path separator, ".", "..",
// "...", and other special characters.
inline std::string FileIdToFileName(const std::string& fileId)
{
    constexpr char hex[] = "0123456789abcdef";
    std::string fileName = "file-";
    for (const char ch : fileId)
    {
        const auto byte = static_cast<std::uint8_t>(ch);
        fileName.push_back(hex[byte / 16]);
        fileName.push_back(hex[byte % 16]);
    }
    return fileName;
}
} // namespace SFS::sample
