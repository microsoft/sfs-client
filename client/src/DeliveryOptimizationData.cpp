// Copyright (c) Microsoft Corporation. All rights reserved.

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
    out->m_description = std::move(description);
    out->m_catalogId = std::move(catalogId);
    out->m_properties = std::move(properties);

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