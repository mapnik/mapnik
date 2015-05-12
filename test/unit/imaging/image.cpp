#include "catch.hpp"

// mapnik
#include <mapnik/image_any.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_view_any.hpp>
#include <mapnik/image_util.hpp>

TEST_CASE("image class") {

SECTION("test gray16") {
    
    const mapnik::image_gray16 im(4,4);
    mapnik::image_gray16 im2(im);
    mapnik::image_gray16 im3(5,5);

    CHECK(im == im);
    CHECK_FALSE(im == im2);
    CHECK_FALSE(im2 == im3);
    CHECK(im < im3);
    CHECK_FALSE(im < im2);
    
    // Check that width is correct
    CHECK(im.width() == 4);
    CHECK(im2.width() == 4);
    
    // Check that height is correct
    CHECK(im.height() == 4);
    CHECK(im2.height() == 4);

    CHECK(im(0,0) == 0);
    CHECK(im2(0,0) == 0);
    im2(0,0) = 1;
    CHECK(im2(0,0) == 1);
    im2.set(514);
    CHECK(im2(0,0) == 514);
    CHECK(im2(1,1) == 514);

    // Check that size is correct
    CHECK(im.size() == 32);
    CHECK(im2.size() == 32);
    
    // Check that row_size is correct
    CHECK(im.row_size() == 8);
    CHECK(im2.row_size() == 8);
    
    // Check that get_premultiplied is correct
    CHECK_FALSE(im.get_premultiplied());
    CHECK_FALSE(im2.get_premultiplied());

    // Check that set premultiplied works
    im2.set_premultiplied(true);
    CHECK(im2.get_premultiplied());
    
    // Check that painted is correct
    CHECK_FALSE(im.painted());
    CHECK_FALSE(im2.painted());

    // Check that set premultiplied works
    im2.painted(true);
    CHECK(im2.painted());

    // Check that offset is correct
    CHECK(im.get_offset() == 0.0);
    CHECK(im2.get_offset() == 0.0);
    
    // Check that set offset works
    im2.set_offset(2.3);
    CHECK(im2.get_offset() == 2.3);
    
    // Check that scaling is correct
    CHECK(im.get_scaling() == 1.0);
    CHECK(im2.get_scaling() == 1.0);
    
    // Check that set scaling works
    im2.set_scaling(1.1);
    CHECK(im2.get_scaling() == 1.1);

    // CHECK that image dtype is correct
    CHECK(im.get_dtype() == mapnik::image_dtype_gray16);
    CHECK(im2.get_dtype() == mapnik::image_dtype_gray16);

    using pixel_type = mapnik::image_view_gray16::pixel_type;
    pixel_type expected_val;
    // Check that all data in the view is correct
    // IM
    expected_val = 0;
    pixel_type const* data_im = im.data();
    CHECK(*data_im == expected_val);
    unsigned char const* data_b = im.bytes();
    CHECK(*data_b == 0);
    for (std::size_t y = 0; y < im.height(); ++y)
    {
        std::size_t width = im.width();
        pixel_type const* data_1  = im.get_row(y);
        pixel_type const* data_2  = im.get_row(y, 1);
        for (std::size_t x = 0; x < width; ++x)
        {
            CHECK(*data_1 == expected_val);
            ++data_1;
        }
        for (std::size_t x = 1; x < width; ++x)
        {
            CHECK(*data_2 == expected_val);
            ++data_2;
        }
    }
    // IM2
    expected_val = 514;
    pixel_type * data_im2 = im2.data();
    CHECK(*data_im2 == expected_val);
    unsigned char * data_b2 = im2.bytes();
    CHECK(*data_b2 == 2);
    ++data_b;
    CHECK(*data_b2 == 2);
    for (std::size_t y = 0; y < im2.height(); ++y)
    {
        std::size_t width = im2.width();
        pixel_type const* data_1  = im2.get_row(y);
        pixel_type const* data_2  = im2.get_row(y, 1);
        for (std::size_t x = 0; x < width; ++x)
        {
            CHECK(*data_1 == expected_val);
            ++data_1;
        }
        for (std::size_t x = 1; x < width; ++x)
        {
            CHECK(*data_2 == expected_val);
            ++data_2;
        }
    }

    // Test set row
    std::vector<pixel_type> v1(im2.width(), 30);
    std::vector<pixel_type> v2(im2.width()-1, 50);
    im2.set_row(0, v1.data(), v1.size());
    im2.set_row(1, 1, v2.size(), v2.data());

    CHECK(im2(0,0) == 30);
    CHECK(im2(0,1) == 514);
    CHECK(im2(1,1) == 50);
    
} // END SECTION

SECTION("image_null")
{
    mapnik::image_null im_null;
/*    const mapnik::image_view_null view_null2;
    mapnik::image_view_null view_null3(view_null2);
    mapnik::image_view_null & view_null4 = view_null3;
    
    // All nulls are equal
    CHECK(view_null == view_null4);
    CHECK(view_null == view_null2);
    
    // No null is greater
    CHECK_FALSE(view_null < view_null4);
    CHECK_FALSE(view_null < view_null2);

    // Check defaults
    CHECK(view_null.x() == 0);
    CHECK(view_null.y() == 0);
    CHECK(view_null.width() == 0);
    CHECK(view_null.height() == 0);
    CHECK(view_null.size() == 0);
    CHECK(view_null.row_size() == 0);
    CHECK(view_null.get_offset() == 0.0);
    CHECK(view_null.get_scaling() == 1.0);
    CHECK(view_null.get_dtype() == mapnik::image_dtype_null);
    CHECK_FALSE(view_null.get_premultiplied());

    // Should throw if we try to access data.
    REQUIRE_THROWS(view_null(0,0));

    CHECK(view_null.get_row(0) == nullptr);
    CHECK(view_null.get_row(0,0) == nullptr);
  */  
} // END SECTION

SECTION("image any")
{
    /*
    mapnik::image_view_any im_any_null;
    CHECK(im_any_null.get_dtype() == mapnik::image_dtype_null);

    mapnik::image_gray8 im(4,4);
    mapnik::image_view_gray8 im_view(0,0,4,4,im);
    mapnik::image_view_any im_view_any(im_view);

    CHECK(im_view_any.get_dtype() == mapnik::image_dtype_gray8);
    CHECK(im_view_any.width() == 4);
    CHECK(im_view_any.height() == 4);
    CHECK(im_view_any.size() == 16);
    CHECK(im_view_any.row_size() == 4);
    CHECK_FALSE(im_view_any.get_premultiplied());
    CHECK(im_view_any.get_offset() == 0.0);
    CHECK(im_view_any.get_scaling() == 1.0);
*/
} // END SECTION

} // END TEST CASE

