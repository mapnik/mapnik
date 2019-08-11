
#include "catch.hpp"

// mapnik
#include <mapnik/image_any.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_util.hpp>

TEST_CASE("image premultiply") {

SECTION("test rgba8") {

    mapnik::image_rgba8 im(4,4);
    mapnik::image_rgba8 im2(4,4,true,true); // Initialize as already premultiplied
    mapnik::image_any im_any(mapnik::image_rgba8(4,4));
    mapnik::image_any im2_any(mapnik::image_rgba8(4,4,true,true));
    
    // First test that the default state is correct for each
    CHECK_FALSE(im.get_premultiplied());
    CHECK_FALSE(im_any.get_premultiplied());
    CHECK(im2.get_premultiplied());
    CHECK(im2_any.get_premultiplied());

    // Set the image to premultiplied
    im.set_premultiplied(true);
    CHECK(im.get_premultiplied());
    // Set back to not premultiplied
    im.set_premultiplied(false);
    CHECK_FALSE(im.get_premultiplied());
    
    mapnik::set_premultiplied_alpha(im, true);
    CHECK(im.get_premultiplied());
    mapnik::set_premultiplied_alpha(im, false);
    CHECK_FALSE(im.get_premultiplied());

    // Fill the images with meaningfull values
    mapnik::color c1(57,70,128,128); // This color is not premultiplied
    mapnik::color c2(57,70,128,128, true); // This color is premultiplied
    mapnik::fill(im, c2); // Because c2 is premultiplied it will make the image premultiplied
    mapnik::fill(im_any, c2); // Because c2 is premultiplied it will make the image premultiplied
    mapnik::fill(im2, c1); // Because c1 is not premultiplied it will make the image not premultiplied
    mapnik::fill(im2_any, c1); // Because c1 is not premultiplied it will make the image not premultiplied

    // Demultipled via image_util
    CHECK(mapnik::demultiply_alpha(im)); // Should return true as was premultiplied
    CHECK(mapnik::demultiply_alpha(im_any)); // Should return true as was premultiplied
    CHECK_FALSE(mapnik::demultiply_alpha(im2)); // Should return false as was demultiplied
    CHECK_FALSE(mapnik::demultiply_alpha(im2_any)); // Should return false as was demultiplied
    
    mapnik::color out;
    // This will be higher because it became demultiplied!
    out = mapnik::get_pixel<mapnik::color>(im, 0, 0);
    CHECK(static_cast<int>(out.red()) == 113);
    CHECK(static_cast<int>(out.green()) == 139);
    CHECK(static_cast<int>(out.blue()) == 255);
    CHECK(static_cast<int>(out.alpha()) == 128);
    out = mapnik::get_pixel<mapnik::color>(im_any, 0, 0);
    CHECK(static_cast<int>(out.red()) == 113);
    CHECK(static_cast<int>(out.green()) == 139);
    CHECK(static_cast<int>(out.blue()) == 255);
    CHECK(static_cast<int>(out.alpha()) == 128);
    // This will be the same because it was already demultiplied
    out = mapnik::get_pixel<mapnik::color>(im2, 0, 0);
    CHECK(static_cast<int>(out.red()) == 57);
    CHECK(static_cast<int>(out.green()) == 70);
    CHECK(static_cast<int>(out.blue()) == 128);
    CHECK(static_cast<int>(out.alpha()) == 128);
    out = mapnik::get_pixel<mapnik::color>(im2_any, 0, 0);
    CHECK(static_cast<int>(out.red()) == 57);
    CHECK(static_cast<int>(out.green()) == 70);
    CHECK(static_cast<int>(out.blue()) == 128);
    CHECK(static_cast<int>(out.alpha()) == 128);

    // Set back to im2s to "premultiplied" with out changing underlying values
    mapnik::set_premultiplied_alpha(im2, true);
    mapnik::set_premultiplied_alpha(im2_any, true);

    // Demultipled via image_util
    CHECK(mapnik::premultiply_alpha(im)); // Should return true as was demultiplied
    CHECK(mapnik::premultiply_alpha(im_any)); // Should return true as was demultiplied
    CHECK_FALSE(mapnik::premultiply_alpha(im2)); // Should return false as was premultiplied
    CHECK_FALSE(mapnik::premultiply_alpha(im2_any)); // Should return false as was premultiplied

    // This will be the same because it was already demultiplied
    out = mapnik::get_pixel<mapnik::color>(im, 0, 0);
    CHECK(static_cast<int>(out.red()) == 57);
    CHECK(static_cast<int>(out.green()) == 70);
    CHECK(static_cast<int>(out.blue()) == 128);
    CHECK(static_cast<int>(out.alpha()) == 128);
    out = mapnik::get_pixel<mapnik::color>(im_any, 0, 0);
    CHECK(static_cast<int>(out.red()) == 57);
    CHECK(static_cast<int>(out.green()) == 70);
    CHECK(static_cast<int>(out.blue()) == 128);
    CHECK(static_cast<int>(out.alpha()) == 128);
    // This will be the same because it was already demultiplied
    out = mapnik::get_pixel<mapnik::color>(im2, 0, 0);
    CHECK(static_cast<int>(out.red()) == 57);
    CHECK(static_cast<int>(out.green()) == 70);
    CHECK(static_cast<int>(out.blue()) == 128);
    CHECK(static_cast<int>(out.alpha()) == 128);
    out = mapnik::get_pixel<mapnik::color>(im2_any, 0, 0);
    CHECK(static_cast<int>(out.red()) == 57);
    CHECK(static_cast<int>(out.green()) == 70);
    CHECK(static_cast<int>(out.blue()) == 128);
    CHECK(static_cast<int>(out.alpha()) == 128);

} // END SECTION

SECTION("test gray8") {
    
    mapnik::image_gray8 im(4,4);
    mapnik::image_gray8 im2(4,4,true,true); // Initialize as already premultiplied
    mapnik::image_any im_any(mapnik::image_gray8(4,4));
    mapnik::image_any im2_any(mapnik::image_gray8(4,4,true,true));
    
    // First test that the default state is correct for each
    CHECK_FALSE(im.get_premultiplied());
    CHECK_FALSE(im_any.get_premultiplied());
    CHECK(im2.get_premultiplied());
    CHECK(im2_any.get_premultiplied());

    // Set the image to premultiplied
    im.set_premultiplied(true);
    CHECK(im.get_premultiplied());
    // Set back to not premultiplied
    im.set_premultiplied(false);
    CHECK_FALSE(im.get_premultiplied());
    
    mapnik::set_premultiplied_alpha(im, true);
    CHECK(im.get_premultiplied());
    mapnik::set_premultiplied_alpha(im, false);
    CHECK_FALSE(im.get_premultiplied());
    
    // Always fails on demultiply since its gray8
    CHECK_FALSE(mapnik::demultiply_alpha(im));
    CHECK_FALSE(mapnik::demultiply_alpha(im_any));
    CHECK_FALSE(mapnik::demultiply_alpha(im2));
    CHECK_FALSE(mapnik::demultiply_alpha(im2_any));
    
    // Always fails on premultiply since its gray8
    CHECK_FALSE(mapnik::premultiply_alpha(im));
    CHECK_FALSE(mapnik::premultiply_alpha(im_any));
    CHECK_FALSE(mapnik::premultiply_alpha(im2));
    CHECK_FALSE(mapnik::premultiply_alpha(im2_any));

} // END SECTION
} // END TEST_CASE
