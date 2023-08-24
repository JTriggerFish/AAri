
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "../src/core/utils/FieldMap.h"

inline constexpr char one[] = "one";
inline constexpr char two[] = "two";
inline constexpr char three[] = "three";

using MyMap = FieldMap<NameArray<one, two, three>, IntArray<1, 2, 3>>;

TEST_CASE("FieldMap works as expected", "[FieldMap]") {
    REQUIRE(MyMap::get("one") == 1);
    REQUIRE(MyMap::get("two") == 2);
    REQUIRE(MyMap::get("three") == 3);
    REQUIRE_THROWS_AS(MyMap::get("four"), std::runtime_error);
}