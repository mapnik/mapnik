
#include "catch.hpp"

// mapnik
#include <mapnik/image_any.hpp>
#include <mapnik/image_view_any.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_util.hpp>


TEST_CASE("image is_solid") {

SECTION("test rgba8") {

    mapnik::image_rgba8 im(4,4);
    mapnik::image_any im_any(mapnik::image_rgba8(4,4));

    CHECK(mapnik::is_solid(im));
    CHECK(mapnik::is_solid(im_any));

    mapnik::image_view_rgba8 im_view(0,0,4,4,im);
    mapnik::image_view_any im_view_any(mapnik::image_view_rgba8(0,0,4,4,im));

    CHECK(mapnik::is_solid(im_view));
    CHECK(mapnik::is_solid(im_view_any));

    mapnik::color c1("green");
    mapnik::color c2("blue");
    mapnik::fill(im, c1);
    mapnik::fill(im_any, c1);
    
    CHECK(mapnik::is_solid(im));
    CHECK(mapnik::is_solid(im_any));
    CHECK(mapnik::is_solid(im_view));
    CHECK(mapnik::is_solid(im_view_any));

    mapnik::set_pixel(im, 0, 0, c2);
    mapnik::set_pixel(im_any, 0, 0, c2);

    CHECK_FALSE(mapnik::is_solid(im));
    CHECK_FALSE(mapnik::is_solid(im_any));
    CHECK_FALSE(mapnik::is_solid(im_view));
    CHECK_FALSE(mapnik::is_solid(im_view_any));

} // END SECTION

SECTION("test gray8") {

    mapnik::image_gray8 im(4,4);
    mapnik::image_any im_any(mapnik::image_gray8(4,4));

    CHECK(mapnik::is_solid(im));
    CHECK(mapnik::is_solid(im_any));

    mapnik::image_view_gray8 im_view(0,0,4,4,im);
    mapnik::image_view_any im_view_any(mapnik::image_view_gray8(0,0,4,4,im));

    CHECK(mapnik::is_solid(im_view));
    CHECK(mapnik::is_solid(im_view_any));

    mapnik::fill(im, 1);
    mapnik::fill(im_any, 1);
    
    CHECK(mapnik::is_solid(im));
    CHECK(mapnik::is_solid(im_any));
    CHECK(mapnik::is_solid(im_view));
    CHECK(mapnik::is_solid(im_view_any));

    mapnik::set_pixel(im, 0, 0, 2);
    mapnik::set_pixel(im_any, 0, 0, 2);

    CHECK_FALSE(mapnik::is_solid(im));
    CHECK_FALSE(mapnik::is_solid(im_any));
    CHECK_FALSE(mapnik::is_solid(im_view));
    CHECK_FALSE(mapnik::is_solid(im_view_any));

} // END SECTION

SECTION("test image null") {

    mapnik::image_null im;
    mapnik::image_any im_any;

    CHECK(mapnik::is_solid(im));
    CHECK(mapnik::is_solid(im_any));

    mapnik::image_view_null im_view;
    mapnik::image_view_any im_view_any;

    CHECK(mapnik::is_solid(im_view));
    CHECK(mapnik::is_solid(im_view_any));

} // END SECTION

} // END TEST CASE
