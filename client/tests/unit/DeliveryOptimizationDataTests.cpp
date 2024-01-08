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

    SECTION("Testing DeliveryOptimizationData equality operators")
    {
        auto CompareData = [&data](const std::string& description,
                                   const std::string& catalogId,
                                   const DOProperties& properties,
                                   bool isEqual) {
            std::unique_ptr<DeliveryOptimizationData> otherData;
            REQUIRE(DeliveryOptimizationData::Make(description, catalogId, properties, otherData) == Result::S_Ok);
            REQUIRE(otherData != nullptr);

            if (isEqual)
            {
                REQUIRE(*data == *otherData);
                REQUIRE_FALSE(*data != *otherData);
            }
            else
            {
                REQUIRE(*data != *otherData);
                REQUIRE_FALSE(*data == *otherData);
            }
        };

        CompareData(description, catalogId, properties, true /*isEqual*/);
        CompareData("", catalogId, properties, false /*isEqual*/);
        CompareData(description, "", properties, false /*isEqual*/);
        CompareData(description, catalogId, {}, false /*isEqual*/);
        CompareData("", "", {}, false /*isEqual*/);
    }
}
