// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/DeliveryOptimizationData.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[DeliveryOptimizationDataTests] " __VA_ARGS__)

using namespace SFS;

TEST("Testing DeliveryOptimizationData::Make()")
{
    std::unique_ptr<DeliveryOptimizationData> data;

    const std::string description{"description"};
    const std::string catalogId{"catalogId"};
    const DOProperties properties{{"key1", "value1"}, {"key2", "value2"}};

    REQUIRE(DeliveryOptimizationData::Make(description, catalogId, properties, data) == Result::S_Ok);
    REQUIRE(data != nullptr);

    CHECK(description == data->GetDescription());
    CHECK(catalogId == data->GetCatalogId());
    CHECK(properties == data->GetProperties());
}
