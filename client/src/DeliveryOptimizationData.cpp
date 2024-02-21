// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "DeliveryOptimizationData.h"

#include "details/ErrorHandling.h"
#include "details/Util.h"

using namespace SFS;
using namespace SFS::details::util;

Result DeliveryOptimizationData::Make(std::string catalogId,
                                      DOProperties properties,
                                      std::unique_ptr<DeliveryOptimizationData>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<DeliveryOptimizationData> tmp(new DeliveryOptimizationData());
    tmp->m_catalogId = std::move(catalogId);
    tmp->m_properties = std::move(properties);

    out = std::move(tmp);
    return Result::Success;
}
SFS_CATCH_RETURN()

DeliveryOptimizationData::DeliveryOptimizationData(DeliveryOptimizationData&& other) noexcept
{
    m_catalogId = std::move(other.m_catalogId);
    m_properties = std::move(other.m_properties);
}

const std::string& DeliveryOptimizationData::GetCatalogId() const noexcept
{
    return m_catalogId;
}

const DOProperties& DeliveryOptimizationData::GetProperties() const noexcept
{
    return m_properties;
}

bool DeliveryOptimizationData::operator==(const DeliveryOptimizationData& other) const noexcept
{
    // String characters can be UTF-8 encoded, so we need to compare them in a case-sensitive manner.
    return m_catalogId == other.m_catalogId && m_properties == other.m_properties;
}

bool DeliveryOptimizationData::operator!=(const DeliveryOptimizationData& other) const noexcept
{
    return !(*this == other);
}
