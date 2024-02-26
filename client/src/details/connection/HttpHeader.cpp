// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HttpHeader.h"

#include <correlation_vector/correlation_vector.h>

std::string SFS::details::ToString(HttpHeader header)
{
    switch (header)
    {
    case HttpHeader::ContentType:
        return "Content-Type";
    case HttpHeader::MSCV:
        return microsoft::correlation_vector::HEADER_NAME;
    }

    return "";
}
