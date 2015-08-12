#include "catch.hpp"

// mapnik
#include <mapnik/value.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_filter.hpp>
#include <mapnik/image_util.hpp>

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

SECTION("test scale-hsla") {
    
    mapnik::image_rgba8 im(3,3);
    mapnik::fill(im,mapnik::color("blue"));
    mapnik::set_pixel(im, 1, 1, mapnik::color("red"));

    // Should throw because a value is greater then 1.0
    REQUIRE_THROWS(mapnik::filter::filter_image(im, "scale-hsla(0.0,1.5,0.0,1.0,0.0,0.5,0.0,0.5)"););

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

    CHECK(im(0,0) == 0xfffc0000);
    CHECK(im(0,1) == 0xfffc0000);
    CHECK(im(0,2) == 0xfffc0000);
    CHECK(im(1,0) == 0xfffc0000);
    CHECK(im(1,1) == 0xfffc0000);
    CHECK(im(1,2) == 0xfffc0000);
    CHECK(im(2,0) == 0xfffc0000);
    CHECK(im(2,1) == 0xfffc0000);
    CHECK(im(2,2) == 0xfffc0000);
    
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

