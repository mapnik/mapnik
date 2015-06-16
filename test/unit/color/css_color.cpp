#include "catch.hpp"
#include <mapnik/css_color_grammar.hpp>
#include <mapnik/safe_cast.hpp>

TEST_CASE("css color") {

    SECTION("conversions")
    {
        mapnik::percent_conv_impl conv;
        CHECK( conv(1.0) == 3 );
        CHECK( conv(60.0) == 153 );
        // should not overflow on invalid input
        CHECK( conv(100000.0) == 255 );
        CHECK( conv(-100000.0) == 0 );

        mapnik::alpha_conv_impl conv2;
        CHECK( conv2(0.5) == 128 );
        CHECK( conv2(1.0) == 255 );
        // should not overflow on invalid input
        CHECK( conv2(60.0) == 255 );
        CHECK( conv2(100000.0) == 255 );
        CHECK( conv2(-100000.0) == 0 );

        mapnik::hsl_conv_impl conv3;
        mapnik::color c;
        conv3(c, 1.0, 1.0, 1.0);
        CHECK( c.alpha() == 255 );
        CHECK( c.red() == 3 );
        CHECK( c.green() == 3 );
        CHECK( c.blue() == 3 );
        // invalid
        conv3(c, -1, -1, -1);
        CHECK( c.alpha() == 255 ); // should not be touched by hsl converter
        CHECK( c.red() == 0 );
        CHECK( c.green() == 0 );
        CHECK( c.blue() == 0 );
    }
}