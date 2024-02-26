// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "HttpHeader.h"

std::string SFS::details::ToString(HttpHeader header)
{
    switch (header)
    {
    case HttpHeader::ContentType:
        return "Content-Type";
    }

    return "";
}
