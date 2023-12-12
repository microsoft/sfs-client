// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "Result.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace SFS
{
using DOProperties = std::unordered_map<std::string, std::string>;

class DeliveryOptimizationData
{
  public:
    [[nodiscard]] static Result Make(std::string description,
                                     std::string catalogId,
                                     DOProperties properties,
                                     std::unique_ptr<DeliveryOptimizationData>& out) noexcept;

    DeliveryOptimizationData(const DeliveryOptimizationData&) = delete;
    DeliveryOptimizationData& operator=(const DeliveryOptimizationData&) = delete;

    const std::string& GetDescription() const noexcept;
    const std::string& GetCatalogId() const noexcept;
    const DOProperties& GetProperties() const noexcept;

  private:
    DeliveryOptimizationData() = default;

    std::string m_description;
    std::string m_catalogId;
    DOProperties m_properties;
};
} // namespace SFS
