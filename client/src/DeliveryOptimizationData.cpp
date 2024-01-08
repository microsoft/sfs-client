// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "DeliveryOptimizationData.h"

#include "details/ErrorHandling.h"

using namespace SFS;

Result DeliveryOptimizationData::Make(std::string description,
                                      std::string catalogId,
                                      DOProperties properties,
                                      std::unique_ptr<DeliveryOptimizationData>& out) noexcept
try
{
    out.reset();

    std::unique_ptr<DeliveryOptimizationData> tmp(new DeliveryOptimizationData());
    tmp->m_description = std::move(description);
    tmp->m_catalogId = std::move(catalogId);
    tmp->m_properties = std::move(properties);

    out = std::move(tmp);
    return Result::S_Ok;
}
SFS_CATCH_RETURN()

const std::string& DeliveryOptimizationData::GetDescription() const noexcept
{
    return m_description;
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
    return m_description == other.m_description && m_catalogId == other.m_catalogId &&
           m_properties == other.m_properties;
}

bool DeliveryOptimizationData::operator!=(const DeliveryOptimizationData& other) const noexcept
{
    return !(*this == other);
}
