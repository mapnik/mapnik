
#include "catch.hpp"

// mapnik
#include <mapnik/image.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_view_any.hpp>
#include <mapnik/image_util.hpp>

TEST_CASE("image view")
{
    SECTION("test rgba8")
    {
        mapnik::image_rgba8 im(4, 4);
        mapnik::color c_red("red");
        mapnik::color c_blue("blue");
        mapnik::color c_green("green");
        mapnik::color c_yellow("yellow");
        mapnik::fill(im, c_red);
        // Upper Left 2x2 is blue
        mapnik::set_pixel(im, 0, 0, c_blue);
        mapnik::set_pixel(im, 0, 1, c_blue);
        mapnik::set_pixel(im, 1, 0, c_blue);
        mapnik::set_pixel(im, 1, 1, c_blue);
        // Upper Right 2x2 is green
        mapnik::set_pixel(im, 2, 0, c_green);
        mapnik::set_pixel(im, 2, 1, c_green);
        mapnik::set_pixel(im, 3, 0, c_green);
        mapnik::set_pixel(im, 3, 1, c_green);
        // Lower Left 2x2 is yellow
        mapnik::set_pixel(im, 0, 2, c_yellow);
        mapnik::set_pixel(im, 0, 3, c_yellow);
        mapnik::set_pixel(im, 1, 2, c_yellow);
        mapnik::set_pixel(im, 1, 3, c_yellow);

        mapnik::image_rgba8 im2(5, 5);
        mapnik::fill(im2, c_red);

        // Now that we have test data run tests
        mapnik::image_view_rgba8 view_all(0, 0, 4, 4, im);
        mapnik::image_view_rgba8 view_blue(0, 0, 2, 2, im);
        mapnik::image_view_rgba8 view_green(2, 0, 2, 2, im);
        mapnik::image_view_rgba8 view_yellow(0, 2, 2, 2, im);
        mapnik::image_view_rgba8 view_red(2, 2, 2, 2, im);
        mapnik::image_view_rgba8 view_bad(99, 99, 99, 99, im);
        const mapnik::image_view_rgba8 view_all_2(0, 0, 4, 4, im2);

        // Check that image_views all have the same underlying data
        CHECK(view_all == view_blue);
        CHECK(view_all == view_green);
        CHECK(view_all == view_yellow);
        CHECK(view_all == view_red);

        CHECK(view_all.data() == im);

        // Check that view_all and view_all_2 are not the same underlying data
        CHECK_FALSE(view_all == view_all_2);
        CHECK(view_all < view_all_2);

        // Check that copy constructor works
        mapnik::image_view_rgba8 view_all_3(view_all_2);
        CHECK(view_all_2 == view_all_3);

        // Check other constructor
        mapnik::image_view_rgba8 view_all_4(std::move(view_all_3));
        CHECK(view_all_2 == view_all_4);

        // Check that x offset is correct
        CHECK(view_all.x() == 0);
        CHECK(view_blue.x() == 0);
        CHECK(view_green.x() == 2);
        CHECK(view_yellow.x() == 0);
        CHECK(view_red.x() == 2);
        CHECK(view_bad.x() == 3);

        // Check that y offset is correct
        CHECK(view_all.y() == 0);
        CHECK(view_blue.y() == 0);
        CHECK(view_green.y() == 0);
        CHECK(view_yellow.y() == 2);
        CHECK(view_red.y() == 2);
        CHECK(view_bad.y() == 3);

        // Check that width is correct
        CHECK(view_all.width() == 4);
        CHECK(view_blue.width() == 2);
        CHECK(view_green.width() == 2);
        CHECK(view_yellow.width() == 2);
        CHECK(view_red.width() == 2);
        CHECK(view_bad.width() == 1);

        // Check that height is correct
        CHECK(view_all.height() == 4);
        CHECK(view_blue.height() == 2);
        CHECK(view_green.height() == 2);
        CHECK(view_yellow.height() == 2);
        CHECK(view_red.height() == 2);
        CHECK(view_bad.height() == 1);

        // Check that size is correct
        CHECK(view_all.size() == 64);
        CHECK(view_blue.size() == 16);
        CHECK(view_green.size() == 16);
        CHECK(view_yellow.size() == 16);
        CHECK(view_red.size() == 16);

        // Check that row_size is correct
        CHECK(view_all.row_size() == 16);
        CHECK(view_blue.row_size() == 8);
        CHECK(view_green.row_size() == 8);
        CHECK(view_yellow.row_size() == 8);
        CHECK(view_red.row_size() == 8);

        // Check that get_premultiplied is correct
        CHECK_FALSE(view_all.get_premultiplied());
        CHECK_FALSE(view_blue.get_premultiplied());
        CHECK_FALSE(view_green.get_premultiplied());
        CHECK_FALSE(view_yellow.get_premultiplied());
        CHECK_FALSE(view_red.get_premultiplied());

        // Check that operator to retrieve value works properly
        CHECK(view_all(0, 0) == c_blue.rgba());
        CHECK(view_blue(0, 0) == c_blue.rgba());
        CHECK(view_green(0, 0) == c_green.rgba());
        CHECK(view_yellow(0, 0) == c_yellow.rgba());
        CHECK(view_red.row_size() == 8);

        // Check that offset is correct
        CHECK(view_all.get_offset() == 0.0);
        CHECK(view_blue.get_offset() == 0.0);
        CHECK(view_green.get_offset() == 0.0);
        CHECK(view_yellow.get_offset() == 0.0);
        CHECK(view_red.get_offset() == 0.0);

        // Check that scaling is correct
        CHECK(view_all.get_scaling() == 1.0);
        CHECK(view_blue.get_scaling() == 1.0);
        CHECK(view_green.get_scaling() == 1.0);
        CHECK(view_yellow.get_scaling() == 1.0);
        CHECK(view_red.get_scaling() == 1.0);

        // CHECK that image dtype is correct
        CHECK(view_all.get_dtype() == mapnik::image_dtype_rgba8);
        CHECK(view_blue.get_dtype() == mapnik::image_dtype_rgba8);
        CHECK(view_green.get_dtype() == mapnik::image_dtype_rgba8);
        CHECK(view_yellow.get_dtype() == mapnik::image_dtype_rgba8);
        CHECK(view_red.get_dtype() == mapnik::image_dtype_rgba8);

        unsigned expected_val;
        using pixel_type = mapnik::image_view_rgba8::pixel_type;
        // Check that all data in the view is correct
        // Blue
        expected_val = c_blue.rgba();
        for (std::size_t y = 0; y < view_blue.height(); ++y)
        {
            std::size_t width = view_blue.width();
            pixel_type const* data_1 = view_blue.get_row(y);
            pixel_type const* data_2 = view_blue.get_row(y, 1);
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
        // Green
        expected_val = c_green.rgba();
        for (std::size_t y = 0; y < view_green.height(); ++y)
        {
            std::size_t width = view_green.width();
            pixel_type const* data_1 = view_green.get_row(y);
            pixel_type const* data_2 = view_green.get_row(y, 1);
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
        // Yellow
        expected_val = c_yellow.rgba();
        for (std::size_t y = 0; y < view_yellow.height(); ++y)
        {
            std::size_t width = view_yellow.width();
            pixel_type const* data_1 = view_yellow.get_row(y);
            pixel_type const* data_2 = view_yellow.get_row(y, 1);
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
        // Red
        expected_val = c_red.rgba();
        for (std::size_t y = 0; y < view_red.height(); ++y)
        {
            std::size_t width = view_red.width();
            pixel_type const* data_1 = view_red.get_row(y);
            pixel_type const* data_2 = view_red.get_row(y, 1);
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

    } // END SECTION

    SECTION("image_view_null")
    {
        mapnik::image_view_null view_null;
        const mapnik::image_view_null view_null2;
        mapnik::image_view_null view_null3(view_null2);
        mapnik::image_view_null& view_null4 = view_null3;

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
        REQUIRE_THROWS(view_null(0, 0));

        CHECK(view_null.get_row(0) == nullptr);
        CHECK(view_null.get_row(0, 0) == nullptr);

    } // END SECTION

    SECTION("image view any")
    {
        mapnik::image_view_any im_any_null;
        CHECK(im_any_null.get_dtype() == mapnik::image_dtype_null);

        mapnik::image_gray8 im(4, 4);
        mapnik::image_view_gray8 im_view(0, 0, 4, 4, im);
        mapnik::image_view_any im_view_any(im_view);

        CHECK(im_view_any.get_dtype() == mapnik::image_dtype_gray8);
        CHECK(im_view_any.width() == 4);
        CHECK(im_view_any.height() == 4);
        CHECK(im_view_any.size() == 16);
        CHECK(im_view_any.row_size() == 4);
        CHECK_FALSE(im_view_any.get_premultiplied());
        CHECK(im_view_any.get_offset() == 0.0);
        CHECK(im_view_any.get_scaling() == 1.0);

    } // END SECTION

} // END TEST CASE
