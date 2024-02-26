// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

namespace SFS::details
{
enum class HttpHeader
{
    ContentType,
    MSCV,
};

std::string ToString(HttpHeader header);
} // namespace SFS::details
