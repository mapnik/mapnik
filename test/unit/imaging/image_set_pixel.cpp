
#include "catch.hpp"

// mapnik
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>


TEST_CASE("image set_pixel") {

SECTION("test gray32") {
    mapnik::image_gray32 im(256,256);
    mapnik::set_pixel(im, 0, 0, -1);
    auto pixel = mapnik::get_pixel<mapnik::image_gray32::pixel_type>(im, 0, 0);
    INFO( pixel );
    CHECK( pixel == 0 );
}

SECTION("test gray8s") {
    mapnik::image_gray8s im(256,256);
    mapnik::set_pixel(im, 0, 0, std::numeric_limits<mapnik::image_gray8s::pixel_type>::max()+1);
    auto pixel = mapnik::get_pixel<mapnik::image_gray8s::pixel_type>(im, 0, 0);
    INFO( pixel );
    CHECK( (int)pixel == (int)std::numeric_limits<mapnik::image_gray8s::pixel_type>::max() );
}

}