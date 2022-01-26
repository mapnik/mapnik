
#include "catch.hpp"

// mapnik
#include <mapnik/image_any.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_util.hpp>

TEST_CASE("image apply_opacity")
{
    SECTION("test rgba8")
    {
        mapnik::image_rgba8 im(4, 4);
        mapnik::image_rgba8 im2(4, 4, true, true); // Initialize as already premultiplied
        mapnik::image_any im_any(mapnik::image_rgba8(4, 4));
        mapnik::image_any im2_any(mapnik::image_rgba8(4, 4, true, true));

        // Fill the images with meaningfull values
        mapnik::color c1(57, 70, 128, 128);       // This color is not premultiplied
        mapnik::color c2(57, 70, 128, 128, true); // This color is premultiplied
        mapnik::fill(im, c1);      // Because c1 is not premultiplied it will make the image not premultiplied
        mapnik::fill(im_any, c1);  // Because c1 is not premultiplied it will make the image not premultiplied
        mapnik::fill(im2, c2);     // Because c1 is premultiplied it will make the image premultiplied
        mapnik::fill(im2_any, c2); // Because c1 is premultiplied it will make the image premultiplied

        mapnik::apply_opacity(im, 0.75);
        mapnik::apply_opacity(im_any, 0.75);
        mapnik::apply_opacity(im2, 0.75);
        mapnik::apply_opacity(im2_any, 0.75);

        mapnik::color out;
        // This should have only changed the alpha, as it was not premultipleid
        out = mapnik::get_pixel<mapnik::color>(im, 0, 0);
        CHECK(static_cast<int>(out.red()) == 57);
        CHECK(static_cast<int>(out.green()) == 70);
        CHECK(static_cast<int>(out.blue()) == 128);
        CHECK(static_cast<int>(out.alpha()) == 96);
        out = mapnik::get_pixel<mapnik::color>(im_any, 0, 0);
        CHECK(static_cast<int>(out.red()) == 57);
        CHECK(static_cast<int>(out.green()) == 70);
        CHECK(static_cast<int>(out.blue()) == 128);
        CHECK(static_cast<int>(out.alpha()) == 96);
        // This will be different because it is demultiplied then premultiplied again after setting alpha
        out = mapnik::get_pixel<mapnik::color>(im2, 0, 0);
        CHECK(static_cast<int>(out.red()) == 43);
        CHECK(static_cast<int>(out.green()) == 53);
        CHECK(static_cast<int>(out.blue()) == 96);
        CHECK(static_cast<int>(out.alpha()) == 96);
        out = mapnik::get_pixel<mapnik::color>(im2_any, 0, 0);
        CHECK(static_cast<int>(out.red()) == 43);
        CHECK(static_cast<int>(out.green()) == 53);
        CHECK(static_cast<int>(out.blue()) == 96);
        CHECK(static_cast<int>(out.alpha()) == 96);

    } // END SECTION

    SECTION("test rgba8 overflow")
    {
        mapnik::image_rgba8 im(4, 4);
        mapnik::color c(128, 128, 128, 128); // This color is premultiplied
        mapnik::fill(im, c);                 // Because c1 is not premultiplied it will make the image not premultiplied
        mapnik::color out;
        out = mapnik::get_pixel<mapnik::color>(im, 0, 0);
        CHECK(static_cast<int>(out.red()) == 128);
        CHECK(static_cast<int>(out.green()) == 128);
        CHECK(static_cast<int>(out.blue()) == 128);
        CHECK(static_cast<int>(out.alpha()) == 128);

        mapnik::apply_opacity(im, 2.5);

        out = mapnik::get_pixel<mapnik::color>(im, 0, 0);
        CHECK(static_cast<int>(out.red()) == 128);
        CHECK(static_cast<int>(out.green()) == 128);
        CHECK(static_cast<int>(out.blue()) == 128);
        CHECK(static_cast<int>(out.alpha()) == 128);

    } // END SECTION

    SECTION("test rgba8 underflow")
    {
        mapnik::image_rgba8 im(4, 4);
        mapnik::color c(128, 128, 128, 128); // This color is premultiplied
        mapnik::fill(im, c);                 // Because c1 is not premultiplied it will make the image not premultiplied

        mapnik::apply_opacity(im, -2.5);

        mapnik::color out;
        out = mapnik::get_pixel<mapnik::color>(im, 0, 0);
        CHECK(static_cast<int>(out.red()) == 128);
        CHECK(static_cast<int>(out.green()) == 128);
        CHECK(static_cast<int>(out.blue()) == 128);
        CHECK(static_cast<int>(out.alpha()) == 0);

    } // END SECTION

    SECTION("test gray8")
    {
        mapnik::image_gray8 im(4, 4);
        mapnik::image_any im_any(mapnik::image_gray8(4, 4));

        CHECK_THROWS(mapnik::apply_opacity(im, 0.25));
        CHECK_THROWS(mapnik::apply_opacity(im_any, 0.25));

    } // END SECTION
} // END TEST_CASE
