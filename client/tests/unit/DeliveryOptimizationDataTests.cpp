// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "ContentUtil.h"
#include "DeliveryOptimizationData.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[DeliveryOptimizationDataTests] " __VA_ARGS__)

using namespace SFS;
using namespace SFS::details::contentutil;

namespace
{
std::unique_ptr<DeliveryOptimizationData> GetData(const std::string& catalogId, const DOProperties& properties)
{
    std::unique_ptr<DeliveryOptimizationData> data;
    REQUIRE(DeliveryOptimizationData::Make(catalogId, properties, data) == Result::Success);
    REQUIRE(data != nullptr);
    return data;
};
} // namespace

TEST("Testing DeliveryOptimizationData::Make()")
{
    const std::string catalogId{"catalogId"};
    const DOProperties properties{{"key1", "value1"}, {"key2", "value2"}};

    const std::unique_ptr<DeliveryOptimizationData> data = GetData(catalogId, properties);

    CHECK(catalogId == data->GetCatalogId());
    CHECK(properties == data->GetProperties());

    SECTION("Testing DeliveryOptimizationData equality operators")
    {
        SECTION("Equal")
        {
            auto CompareDataEqual = [&data](const std::unique_ptr<DeliveryOptimizationData>& sameData) {
                REQUIRE((*data == *sameData));
                REQUIRE_FALSE((*data != *sameData));
            };

            CompareDataEqual(GetData(catalogId, properties));
        }

        SECTION("Not equal")
        {
            auto CompareDataNotEqual = [&data](const std::unique_ptr<DeliveryOptimizationData>& otherData) {
                REQUIRE((*data != *otherData));
                REQUIRE_FALSE((*data == *otherData));
            };

            CompareDataNotEqual(GetData("", properties));
            CompareDataNotEqual(GetData(catalogId, {}));
            CompareDataNotEqual(GetData("", {}));
            CompareDataNotEqual(GetData("CATALOGID", properties));
        }
    }
}
