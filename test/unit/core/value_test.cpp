
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
        auto add6 = v2 + v2; // result_integer
        // check type promotion
        CHECK(add0.is<mapnik::value_double>());
        CHECK(add1.is<mapnik::value_double>());
        CHECK(add2.is<mapnik::value_double>());
        CHECK(add3.is<mapnik::value_double>());
        CHECK(add4.is<mapnik::value_integer>());
        CHECK(add5.is<mapnik::value_integer>());
        CHECK(add6.is<mapnik::value_integer>());
        //
        CHECK(add6 == v0 + v0);
        // check commutative rules
        CHECK(add0 == add1);
        CHECK(add2 == add3);
        CHECK(add4 == add5);

        // sub
        auto sub0 = v0 - v1; // result value_double
        auto sub1 = v1 - v0;
        auto sub2 = v1 - v2; // result value_double
        auto sub3 = v2 - v1;
        auto sub4 = v0 - v2; // result value_integer
        auto sub5 = v2 - v0;
        auto sub6 = v2 - v2; // result value_integer

        CHECK(sub0.is<mapnik::value_double>());
        CHECK(sub1.is<mapnik::value_double>());
        CHECK(sub2.is<mapnik::value_double>());
        CHECK(sub3.is<mapnik::value_double>());
        CHECK(sub4.is<mapnik::value_integer>());
        CHECK(sub5.is<mapnik::value_integer>());
        CHECK(sub6.is<mapnik::value_integer>());

        // check commutative rules
        CHECK(sub0 == -sub1);
        CHECK(sub2 == -sub3);
        CHECK(sub4 == -sub5);
        CHECK(sub6 == v0 - v0);

        // multl
        auto mult0 = v0 * v1; // result value_double
        auto mult1 = v1 * v0;
        auto mult2 = v1 * v2; // result value_double
        auto mult3 = v2 * v1;
        auto mult4 = v0 * v2; // result value_integer
        auto mult5 = v2 * v0;
        auto mult6 = v2 * v2; // result value_integer

        CHECK(mult0.is<mapnik::value_double>());
        CHECK(mult1.is<mapnik::value_double>());
        CHECK(mult2.is<mapnik::value_double>());
        CHECK(mult3.is<mapnik::value_double>());
        CHECK(mult4.is<mapnik::value_integer>());
        CHECK(mult5.is<mapnik::value_integer>());
        CHECK(mult6.is<mapnik::value_integer>());
        // check commutative rules
        CHECK(mult0 == mult1);
        CHECK(mult2 == mult3);
        CHECK(mult4 == mult5);
        //
        CHECK(mult6 == v0 * v0);

        // div
        auto div0 = v0 / v1; // result value_double
        auto div1 = v1 / v0;
        auto div2 = v1 / v2; // result value_double
        auto div3 = v2 / v1;
        auto div4 = v0 / v2; // result value_integer
        auto div5 = v2 / v0;
        auto div6 = v2 / v2; // result value_interger

        CHECK(div0.is<mapnik::value_double>());
        CHECK(div1.is<mapnik::value_double>());
        CHECK(div2.is<mapnik::value_double>());
        CHECK(div3.is<mapnik::value_double>());
        CHECK(div4.is<mapnik::value_integer>());
        CHECK(div5.is<mapnik::value_integer>());
        CHECK(div6.is<mapnik::value_integer>());

        CHECK(div0 == 1.0/div1);
        CHECK(div2 == 1.0/div3);
        CHECK(div4 == 1.0/div5);
        CHECK(div6 == v0/v0);
    }
}
