// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "sfsclient/DeliveryOptimizationData.h"

#include <catch2/catch_test_macros.hpp>

#define TEST(...) TEST_CASE("[DeliveryOptimizationDataTests] " __VA_ARGS__)

using namespace SFS;

namespace
{
std::unique_ptr<DeliveryOptimizationData> GetData(const std::string& description,
                                                  const std::string& catalogId,
                                                  const DOProperties& properties)
{
    std::unique_ptr<DeliveryOptimizationData> data;
    REQUIRE(DeliveryOptimizationData::Make(description, catalogId, properties, data) == Result::S_Ok);
    REQUIRE(data != nullptr);
    return data;
};
} // namespace

TEST("Testing DeliveryOptimizationData::Make()")
{
    const std::string description{"description"};
    const std::string catalogId{"catalogId"};
    const DOProperties properties{{"key1", "value1"}, {"key2", "value2"}};

    const std::unique_ptr<DeliveryOptimizationData> data = GetData(description, catalogId, properties);

    CHECK(description == data->GetDescription());
    CHECK(catalogId == data->GetCatalogId());
    CHECK(properties == data->GetProperties());

    SECTION("Testing DeliveryOptimizationData equality operators")
    {
        SECTION("Equal")
        {
            auto sameData = GetData(description, catalogId, properties);
            REQUIRE(*data == *sameData);
            REQUIRE_FALSE(*data != *sameData);
        }

        SECTION("Not equal")
        {
            auto CompareDataNotEqual = [&data](const std::unique_ptr<DeliveryOptimizationData>& otherData) {
                REQUIRE(*data != *otherData);
                REQUIRE_FALSE(*data == *otherData);
            };

            CompareDataNotEqual(GetData("", catalogId, properties));
            CompareDataNotEqual(GetData(description, "", properties));
            CompareDataNotEqual(GetData(description, catalogId, {}));
            CompareDataNotEqual(GetData("", "", {}));
        }
    }
}
