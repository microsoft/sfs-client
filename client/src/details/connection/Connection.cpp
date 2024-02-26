// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "Connection.h"

using namespace SFS;
using namespace SFS::details;

Connection::Connection(const ReportingHandler& handler)
    : m_handler(handler), m_cv(handler)
{
}

std::string Connection::Post(const std::string& url)
{
    return Post(url, {});
}

void Connection::SetCorrelationVector(const std::string& cv)
{
    m_cv.SetCorrelationVector(cv);
}
