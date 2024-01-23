// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "CurlConnection.h"

using namespace SFS::details;

CurlConnection::CurlConnection(const ReportingHandler& handler) : Connection(handler)
{
}

CurlConnection::~CurlConnection()
{
}
