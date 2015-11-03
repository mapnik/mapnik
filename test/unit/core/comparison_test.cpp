#include "catch.hpp"

#include <mapnik/value_types.hpp>
#include <mapnik/value.hpp>

TEST_CASE("comparison")
{
    SECTION("operator==()")
    {
        mapnik::value v0 = 1; // mapnik::value_integer
        mapnik::value v1 = 1.001; // mapnik::value_double
        mapnik::value v2 = true; // mapnik::value_boolean

        REQUIRE(!(v0 == v1));
        REQUIRE(!(v1 == v0));

        REQUIRE(!(v1 == v2));
        REQUIRE(!(v2 == v1));

        REQUIRE(v2 == v0);
        REQUIRE(v0 == v2);
    }

    SECTION("operator!=()")
    {
        mapnik::value v0 = 1; // mapnik::value_integer
        mapnik::value v1 = 1.001; // mapnik::value_double
        mapnik::value v2 = true; // mapnik::value_boolean
        mapnik::value v3 = mapnik::value_null(); //

        REQUIRE(v0 != v1);
        REQUIRE(v1 != v0);

        REQUIRE(v1 != v2);
        REQUIRE(v2 != v1);

        REQUIRE(!(v2 != v0));
        REQUIRE(!(v0 != v2));

        REQUIRE(v3 != v0);
        REQUIRE(v3 != v1);
        REQUIRE(v3 != v2);
        REQUIRE(v0 != v3);
        REQUIRE(v1 != v3);
        REQUIRE(v2 != v3);

    }
    SECTION("operator<,<=,>,>=")
    {
        mapnik::value v0 = 1;
        mapnik::value v1 = 1.01;
        mapnik::value v2 = true;
        mapnik::value v3 = 2;
        // value_integer | value_double
        // 1 < 1.01 => true
        // 1.01 > 1 => true
        REQUIRE(v0 < v1);
        REQUIRE(v1 > v0);
        // 1 <= 1.01 => true
        // 1.01 >= 1 => true
        REQUIRE(v0 <= v1);
        REQUIRE(v1 >= v0);

        // value_bool | value_integer
        // true < 1 => false
        // true > 1 => false
        REQUIRE(!(v2 < v0));
        REQUIRE(!(v2 > v0));
        // true <= 1 => true
        // true >= 1 => true
        REQUIRE(v2 <= v0);
        REQUIRE(v2 >= v0);
        // 1 > true => false
        // 1 < true => false
        REQUIRE(!(v0 > v2));
        REQUIRE(!(v0 < v2));
        // 1 >= true => true
        // 1 <= true => true
        REQUIRE(v0 >= v2);
        REQUIRE(v0 <= v2);

        // value_bool | value_doble
        // true < 1.01 => true
        // 1.01 > true => true
        REQUIRE(v2 < v1);
        REQUIRE(v1 > v2);
        // true <= 1.01 => true
        // 1.01 >= true => true
        REQUIRE(v2 <= v1);
        REQUIRE(v1 >= v2);

        // value_integer | value_integer
        // 1 < 2 => true
        // 2 > 1 => true
        REQUIRE(v0 < v3);
        REQUIRE(v3 > v0);
        // 1 <= 2 => true
        // 2 >= 1 => true
        REQUIRE(v0 <= v3);
        REQUIRE(v3 >= v0);
    }
}
