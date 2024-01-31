// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Util.h"

#include <string>

using namespace SFS::details;

bool util::AreEqualI(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
    {
        return false;
    }

    for (auto itA = a.begin(), itB = b.begin(); itA != a.end() && itB != b.end(); ++itA, ++itB)
    {
        if (std::tolower(*itA) != std::tolower(*itB))
        {
            return false;
        }
    }
    return true;
}

bool util::AreNotEqualI(std::string_view a, std::string_view b)
{
    return !AreEqualI(a, b);
}
