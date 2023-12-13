// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/DeliveryOptimizationData.h"

#include <gtest/gtest.h>

using namespace SFS;

TEST(DeliveryOptimizationDataTests, Make)
{
    std::unique_ptr<DeliveryOptimizationData> data;

    const std::string description{"description"};
    const std::string catalogId{"catalogId"};
    const DOProperties properties{{"key1", "value1"}, {"key2", "value2"}};

    ASSERT_EQ(DeliveryOptimizationData::Make(description, catalogId, properties, data).GetCode(), Result::S_Ok);
    ASSERT_NE(nullptr, data);

    EXPECT_EQ(description, data->GetDescription());
    EXPECT_EQ(catalogId, data->GetCatalogId());
    EXPECT_EQ(properties, data->GetProperties());
}
