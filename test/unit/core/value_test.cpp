#include "catch.hpp"

#include <mapnik/value_types.hpp>
#include <mapnik/value.hpp>

TEST_CASE("mapnik::value")
{
    SECTION("add/sub/mult/div")
    {
        mapnik::value v0 = 1; // mapnik::value_integer
        mapnik::value v1 = 1.001; // mapnik::value_double
        mapnik::value v2 = true; // mapnik::value_boolean

        CHECK(v0.is<mapnik::value_integer>());
        CHECK(v1.is<mapnik::value_double>());
        CHECK(v2.is<mapnik::value_bool>());

        // add
        auto add0 = v0 + v1; // result value_double
        auto add1 = v1 + v0;
        auto add2 = v1 + v2; // result value_double
        auto add3 = v2 + v1;
        auto add4 = v0 + v2; // result value_integer
        auto add5 = v2 + v0;
        // check type promotion
        CHECK(add0.is<mapnik::value_double>());
        CHECK(add1.is<mapnik::value_double>());
        CHECK(add2.is<mapnik::value_double>());
        CHECK(add3.is<mapnik::value_double>());
        CHECK(add4.is<mapnik::value_integer>());
        CHECK(add5.is<mapnik::value_integer>());
        // check commutative rules
        CHECK(add0 == add1);
        CHECK(add2 == add3);
        CHECK(add4 == add5);

        // sub
        auto sub0 = v0 - v1;
        auto sub1 = v1 - v0;
        CHECK(sub0.is<mapnik::value_double>());
        CHECK(sub1.is<mapnik::value_double>());
        CHECK(sub0 == -sub1);

        // multl
        auto mult0 = v0 * v1;
        auto mult1 = v1 * v0;
        CHECK(mult0.is<mapnik::value_double>());
        CHECK(mult1.is<mapnik::value_double>());
        CHECK(mult0 == mult1);

        // div
        auto div0 = v0 / v1;
        auto div1 = v1 / v0;
        CHECK(div0.is<mapnik::value_double>());
        CHECK(div1.is<mapnik::value_double>());
        CHECK(div0 == 1.0/div1);
    }
}
