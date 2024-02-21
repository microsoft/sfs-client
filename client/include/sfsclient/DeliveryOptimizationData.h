// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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
    [[nodiscard]] static Result Make(std::string catalogId,
                                     DOProperties properties,
                                     std::unique_ptr<DeliveryOptimizationData>& out) noexcept;

    DeliveryOptimizationData(DeliveryOptimizationData&&) noexcept;

    DeliveryOptimizationData(const DeliveryOptimizationData&) = delete;
    DeliveryOptimizationData& operator=(const DeliveryOptimizationData&) = delete;

    /**
     * @return File identifier for DO client
     */
    const std::string& GetCatalogId() const noexcept;

    /**
     * @return Optional property bag of opaque key value pairs to be passed to DO client if present
     */
    const DOProperties& GetProperties() const noexcept;

    bool operator==(const DeliveryOptimizationData& other) const noexcept;
    bool operator!=(const DeliveryOptimizationData& other) const noexcept;

  private:
    DeliveryOptimizationData() = default;

    std::string m_catalogId;
    DOProperties m_properties;
};
} // namespace SFS
