#include "catch.hpp"

// mapnik
#include <mapnik/value.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_filter.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_filter_types.hpp>
// stl
#include <sstream>
#include <array>

TEST_CASE("image filter") {

SECTION("test bad filter input") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("red"));

    REQUIRE_THROWS( mapnik::filter::filter_image(im, "foo,asdfasdf()"); );
    REQUIRE_THROWS( mapnik::filter::filter_image(im, "colorize-alpha("); );
    REQUIRE_THROWS( mapnik::filter::filter_image(im, "color-to-alpha(blue"); );
    REQUIRE_THROWS( mapnik::filter::filter_image(im, "color-to-alpha(,blue)"); );
    REQUIRE_THROWS( mapnik::filter::filter_image(im, "colorize-alpha()"); );

    REQUIRE_THROWS(
        mapnik::image_rgba8 const& im2 = im;
        mapnik::image_rgba8 new_im = mapnik::filter::filter_image(im2, "foo");
    );

    CHECK(im(0,0) == 0xffff0000);
    CHECK(im(0,1) == 0xffff0000);
    CHECK(im(0,2) == 0xffff0000);
    CHECK(im(1,0) == 0xffff0000);
    CHECK(im(1,1) == 0xff0000ff);
    CHECK(im(1,2) == 0xffff0000);
    CHECK(im(2,0) == 0xffff0000);
    CHECK(im(2,1) == 0xffff0000);
    CHECK(im(2,2) == 0xffff0000);

} // END SECTION

SECTION("test blur") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("red"));

    mapnik::filter::filter_image(im, "blur");

    CHECK(im(0,0) == 0xffc60038);
    CHECK(im(0,1) == 0xffe2001c);
    CHECK(im(0,2) == 0xffc60038);
    CHECK(im(1,0) == 0xffc60038);
    CHECK(im(1,1) == 0xffe2001c);
    CHECK(im(1,2) == 0xffc60038);
    CHECK(im(2,0) == 0xffc60038);
    CHECK(im(2,1) == 0xffe2001c);
    CHECK(im(2,2) == 0xffc60038);

} // END SECTION

SECTION("test blur constant") {

    mapnik::image_rgba8 im_orig(3,3);
    mapnik::fill(im_orig,mapnik::color("blue"));
    mapnik::set_pixel(im_orig, 1, 1, mapnik::color("red"));

    mapnik::image_rgba8 const& im_new = im_orig;
    mapnik::image_rgba8 im = mapnik::filter::filter_image(im_new, "blur");

    CHECK(im(0,0) == 0xffc60038);
    CHECK(im(0,1) == 0xffe2001c);
    CHECK(im(0,2) == 0xffc60038);
    CHECK(im(1,0) == 0xffc60038);
    CHECK(im(1,1) == 0xffe2001c);
    CHECK(im(1,2) == 0xffc60038);
    CHECK(im(2,0) == 0xffc60038);
    CHECK(im(2,1) == 0xffe2001c);
    CHECK(im(2,2) == 0xffc60038);

} // END SECTION

SECTION("test gray") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("red"));

    mapnik::filter::filter_image(im, "gray");

    CHECK(im(0,0) == 0xff1c1c1c);
    CHECK(im(0,1) == 0xff1c1c1c);
    CHECK(im(0,2) == 0xff1c1c1c);
    CHECK(im(1,0) == 0xff1c1c1c);
    CHECK(im(1,1) == 0xff4c4c4c);
    CHECK(im(1,2) == 0xff1c1c1c);
    CHECK(im(2,0) == 0xff1c1c1c);
    CHECK(im(2,1) == 0xff1c1c1c);
    CHECK(im(2,2) == 0xff1c1c1c);

} // END SECTION

SECTION("test agg stack blur") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("red"));

    mapnik::filter::filter_image(im, "agg-stack-blur(1,1)");

    CHECK(im(0,0) == 0xffef000f);
    CHECK(im(0,1) == 0xffdf001f);
    CHECK(im(0,2) == 0xffef000f);
    CHECK(im(1,0) == 0xffdf001f);
    CHECK(im(1,1) == 0xffbf003f);
    CHECK(im(1,2) == 0xffdf001f);
    CHECK(im(2,0) == 0xffef000f);
    CHECK(im(2,1) == 0xffdf001f);
    CHECK(im(2,2) == 0xffef000f);

} // END SECTION

SECTION("test scale-hsla 1") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("red"));

    mapnik::filter::filter_image(im, "scale-hsla(0.0,0.5,0.0,1.0,0.0,0.5,0.0,0.5)");

    CHECK(im(0,0) == 0x80004000);
    CHECK(im(0,1) == 0x80004000);
    CHECK(im(0,2) == 0x80004000);
    CHECK(im(1,0) == 0x80004000);
    CHECK(im(1,1) == 0x80000040);
    CHECK(im(1,2) == 0x80004000);
    CHECK(im(2,0) == 0x80004000);
    CHECK(im(2,1) == 0x80004000);
    CHECK(im(2,2) == 0x80004000);

} // END SECTION

SECTION("test scale-hsla 2") {

    mapnik::image_rgba8 im(3,3);
    mapnik::set_pixel(im, 0, 0, mapnik::color(255, 0, 0));
    mapnik::set_pixel(im, 0, 1, mapnik::color(147, 112, 219));
    mapnik::set_pixel(im, 0, 2, mapnik::color(128, 128, 128));
    mapnik::set_pixel(im, 1, 0, mapnik::color(72, 209, 204));
    mapnik::set_pixel(im, 1, 1, mapnik::color(218, 112, 214));
    mapnik::set_pixel(im, 1, 2, mapnik::color(30, 144, 255));
    mapnik::set_pixel(im, 2, 0, mapnik::color(238, 130, 238));
    mapnik::set_pixel(im, 2, 1, mapnik::color(154, 205, 50));
    mapnik::set_pixel(im, 2, 2, mapnik::color(160, 82, 45));

    // Should not throw on values out of [0, 1]
    // https://github.com/mapnik/mapnik/issues/3052
    REQUIRE_NOTHROW(mapnik::filter::filter_image(im, "scale-hsla(0.0,1.5,-1.0,1.0,-1.0,2.0,1.0,1.0)"););

    CHECK(im(0,0) == 0xff0000ff);
    CHECK(im(0,1) == 0xffefeff4);
    CHECK(im(0,2) == 0xff818181);
    CHECK(im(1,0) == 0xffb895a5);
    CHECK(im(1,1) == 0xffededf3);
    CHECK(im(1,2) == 0xffd75aff);
    CHECK(im(2,0) == 0xffffffff);
    CHECK(im(2,1) == 0xff649b64);
    CHECK(im(2,2) == 0xff2e343b);

} // END SECTION

SECTION("test emboss") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("white"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("orange"));

    mapnik::filter::filter_image(im, "emboss");

    CHECK(im(0,0) == 0xff004bff);
    CHECK(im(0,1) == 0xff00a5ff);
    CHECK(im(0,2) == 0xff004bff);
    CHECK(im(1,0) == 0xffffffff);
    CHECK(im(1,1) == 0xff00a5ff);
    CHECK(im(1,2) == 0xffffffff);
    CHECK(im(2,0) == 0xffffffff);
    CHECK(im(2,1) == 0xffffffff);
    CHECK(im(2,2) == 0xffffffff);

} // END SECTION

SECTION("test sharpen") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("gray"));

    mapnik::filter::filter_image(im, "sharpen");

    CHECK(im(0,0) == 0xffff0000);
    CHECK(im(0,1) == 0xffff0000);
    CHECK(im(0,2) == 0xffff0000);
    CHECK(im(1,0) == 0xffff0000);
    CHECK(im(1,1) == 0xff00ffff);
    CHECK(im(1,2) == 0xffff0000);
    CHECK(im(2,0) == 0xffff0000);
    CHECK(im(2,1) == 0xffff0000);
    CHECK(im(2,2) == 0xffff0000);

} // END SECTION

SECTION("test edge detect") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("gray"));

    mapnik::filter::filter_image(im, "edge-detect");

    CHECK(im(0,0) == 0xff000000);
    CHECK(im(0,1) == 0xff008080);
    CHECK(im(0,2) == 0xff000000);
    CHECK(im(1,0) == 0xff00ffff);
    CHECK(im(1,1) == 0xffff0000);
    CHECK(im(1,2) == 0xff00ffff);
    CHECK(im(2,0) == 0xff000000);
    CHECK(im(2,1) == 0xff008080);
    CHECK(im(2,2) == 0xff000000);

} // END SECTION

SECTION("test sobel") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("gray"));

    mapnik::filter::filter_image(im, "sobel");

    CHECK(im(0,0) == 0xfffeffff);
    CHECK(im(0,1) == 0xfffeffff);
    CHECK(im(0,2) == 0xfffeffff);
    CHECK(im(1,0) == 0xff000000);
    CHECK(im(1,1) == 0xff000000);
    CHECK(im(1,2) == 0xff000000);
    CHECK(im(2,0) == 0xfffeffff);
    CHECK(im(2,1) == 0xfffeffff);
    CHECK(im(2,2) == 0xfffeffff);

} // END SECTION

SECTION("test x-gradient") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("gray"));

    mapnik::filter::filter_image(im, "x-gradient");

    CHECK(im(0,0) == 0xff808080);
    CHECK(im(0,1) == 0xffbf4040);
    CHECK(im(0,2) == 0xff808080);
    CHECK(im(1,0) == 0xff808080);
    CHECK(im(1,1) == 0xff808080);
    CHECK(im(1,2) == 0xff808080);
    CHECK(im(2,0) == 0xff808080);
    CHECK(im(2,1) == 0xff41c0c0);
    CHECK(im(2,2) == 0xff808080);

} // END SECTION

SECTION("test y-gradient") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("gray"));

    mapnik::filter::filter_image(im, "y-gradient");

    CHECK(im(0,0) == 0xff808080);
    CHECK(im(0,1) == 0xff808080);
    CHECK(im(0,2) == 0xff808080);
    CHECK(im(1,0) == 0xffbf4040);
    CHECK(im(1,1) == 0xff808080);
    CHECK(im(1,2) == 0xff41c0c0);
    CHECK(im(2,0) == 0xff808080);
    CHECK(im(2,1) == 0xff808080);
    CHECK(im(2,2) == 0xff808080);

} // END SECTION

SECTION("test invert") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("gray"));

    mapnik::filter::filter_image(im, "invert");

    CHECK(im(0,0) == 0xff00ffff);
    CHECK(im(0,1) == 0xff00ffff);
    CHECK(im(0,2) == 0xff00ffff);
    CHECK(im(1,0) == 0xff00ffff);
    CHECK(im(1,1) == 0xff7f7f7f);
    CHECK(im(1,2) == 0xff00ffff);
    CHECK(im(2,0) == 0xff00ffff);
    CHECK(im(2,1) == 0xff00ffff);
    CHECK(im(2,2) == 0xff00ffff);

} // END SECTION

SECTION("test colorize-alpha - one color") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("gray"));

    mapnik::filter::filter_image(im, "colorize-alpha(blue)");

    CHECK(im(0,0) == 0xffff0000);
    CHECK(im(0,1) == 0xffff0000);
    CHECK(im(0,2) == 0xffff0000);
    CHECK(im(1,0) == 0xffff0000);
    CHECK(im(1,1) == 0xffff0000);
    CHECK(im(1,2) == 0xffff0000);
    CHECK(im(2,0) == 0xffff0000);
    CHECK(im(2,1) == 0xffff0000);
    CHECK(im(2,2) == 0xffff0000);

} // END SECTION

SECTION("test colorize-alpha - two color") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("gray"));

    mapnik::filter::filter_image(im, "colorize-alpha(green,blue)");

    CHECK(im(0,0) == 0xfffd0000);
    CHECK(im(0,1) == 0xfffd0000);
    CHECK(im(0,2) == 0xfffd0000);
    CHECK(im(1,0) == 0xfffd0000);
    CHECK(im(1,1) == 0xfffd0000);
    CHECK(im(1,2) == 0xfffd0000);
    CHECK(im(2,0) == 0xfffd0000);
    CHECK(im(2,1) == 0xfffd0000);
    CHECK(im(2,2) == 0xfffd0000);

} // END SECTION

SECTION("test colorize-alpha - one color with transparency") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("#0000ffaa"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("#aaaaaaaa"));

    mapnik::filter::filter_image(im, "colorize-alpha(#0000ff99)");

    CHECK(im(0,0) == 0x66660000);
    CHECK(im(0,1) == 0x66660000);
    CHECK(im(0,2) == 0x66660000);
    CHECK(im(1,0) == 0x66660000);
    CHECK(im(1,1) == 0x66660000);
    CHECK(im(1,2) == 0x66660000);
    CHECK(im(2,0) == 0x66660000);
    CHECK(im(2,1) == 0x66660000);
    CHECK(im(2,2) == 0x66660000);

} // END SECTION

SECTION("test colorize-alpha - two color with transparency") {

    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("#0000ffaa"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("#aaaaaaaa"));

    mapnik::filter::filter_image(im, "colorize-alpha(#0000ff00,#00ff00ff)");

    CHECK(im(0,0) == 0x70264a00);
    CHECK(im(0,1) == 0x70264a00);
    CHECK(im(0,2) == 0x70264a00);
    CHECK(im(1,0) == 0x70264a00);
    CHECK(im(1,1) == 0x70264a00);
    CHECK(im(1,2) == 0x70264a00);
    CHECK(im(2,0) == 0x70264a00);
    CHECK(im(2,1) == 0x70264a00);
    CHECK(im(2,2) == 0x70264a00);

} // END SECTION

SECTION("test parsing image-filters") {

    std::string str = ""; // empty string
    std::vector<mapnik::filter::filter_type> filters;
    CHECK(parse_image_filters(str, filters));
    CHECK(filters.size() == 0);

    std::array<std::string,17> expected = {{ "emboss",
                                             "emboss",
                                             "blur",
                                             "gray",
                                             "edge-detect",
                                             "sobel",
                                             "sharpen",
                                             "x-gradient",
                                             "y-gradient",
                                             "invert",
                                             "color-blind-protanope",
                                             "color-blind-deuteranope",
                                             "color-blind-tritanope",
                                             "agg-stack-blur(1,1)",
                                             "agg-stack-blur(1,1)",
                                             "agg-stack-blur(2,2)",
                                             "agg-stack-blur(2,3)"}};

    str += "emboss emboss() blur,gray ,edge-detect, sobel , , sharpen,,,x-gradient y-gradientinvert";
    str += "color-blind-protanope color-blind-deuteranope color-blind-tritanope agg-stack-blur,agg-stack-blur(),agg-stack-blur(2),agg-stack-blur(2,3)" ;
    CHECK(parse_image_filters(str, filters));
    CHECK(filters.size() == expected.size());
    std::size_t count = 0;
    for (auto const& filter : filters)
    {
        std::stringstream ss;
        ss << filter;
        CHECK (expected[count++] == ss.str());
    }
}

SECTION("test colorize-alpha - parsing correct input") {

    std::string s("colorize-alpha(#0000ff 0%, #00ff00 100%)");
    std::vector<mapnik::filter::filter_type> f;
    REQUIRE(parse_image_filters(s, f));
    mapnik::filter::colorize_alpha const & ca = mapnik::util::get<mapnik::filter::colorize_alpha>(f.front());
    CHECK(ca.size() == 2);

    CHECKED_IF(ca.size() > 0)
    {
        mapnik::filter::color_stop const & s2 = ca[0];
        CHECK( s2.color.alpha() == 0xff );
        CHECK( s2.color.red() == 0x00 );
        CHECK( s2.color.green() == 0x00 );
        CHECK( s2.color.blue() == 0xff );
        CHECK( s2.offset == 0.0 );
    }

    CHECKED_IF(ca.size() > 1)
    {
        mapnik::filter::color_stop const & s2 = ca[1];
        CHECK( s2.color.alpha() == 0xff );
        CHECK( s2.color.red() == 0x00 );
        CHECK( s2.color.green() == 0xff );
        CHECK( s2.color.blue() == 0x00 );
        CHECK( s2.offset == 1.0 );
    }

} // END SECTION

SECTION("test colorize-alpha - parsing incorrect input") {

    std::string s("colorize-alpha(#0000ff 0%, #00ff00 00 100%)");
    std::vector<mapnik::filter::filter_type> f;
    CHECK(!parse_image_filters(s, f));
    CHECK( f.empty() );

} // END SECTION

SECTION("test color-blind-protanope") {

    mapnik::image_rgba8 im(2,2);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 0, 1, mapnik::color("green"));
    mapnik::set_pixel(im, 1, 0, mapnik::color("yellow"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("red"));

    mapnik::filter::filter_image(im, "color-blind-protanope");

    CHECK(im(0,0) == 0xff9a4a00);
    CHECK(im(0,1) == 0xff006e7c);
    CHECK(im(1,0) == 0xffd9f6ff);
    CHECK(im(1,1) == 0xff1d7e8e);

} // END SECTION

SECTION("test color-blind-deuteranope") {

    mapnik::image_rgba8 im(2,2);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 0, 1, mapnik::color("green"));
    mapnik::set_pixel(im, 1, 0, mapnik::color("yellow"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("red"));

    mapnik::filter::filter_image(im, "color-blind-deuteranope");

    CHECK(im(0,0) == 0xff824f00);
    CHECK(im(0,1) == 0xff1c688b);
    CHECK(im(1,0) == 0xffe9f5ff);
    CHECK(im(1,1) == 0xff0077a0);

} // END SECTION

SECTION("test color-blind-tritanope") {

    mapnik::image_rgba8 im(2,2);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 0, 1, mapnik::color("green"));
    mapnik::set_pixel(im, 1, 0, mapnik::color("yellow"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("red"));

    mapnik::filter::filter_image(im, "color-blind-tritanope");

    CHECK(im(0,0) == 0xff595500);
    CHECK(im(0,1) == 0xff80763a);
    CHECK(im(1,0) == 0xfff8f3ff);
    CHECK(im(1,1) == 0xff0017fd);

} // END SECTION

} // END TEST CASE
