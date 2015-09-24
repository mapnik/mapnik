#include "catch.hpp"

#include <mapnik/value_types.hpp>
#include <mapnik/value.hpp>
#include <mapnik/unicode.hpp>

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
}
