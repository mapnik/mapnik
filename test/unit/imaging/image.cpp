
#include "catch.hpp"

// mapnik
#include <mapnik/image.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_util.hpp>

TEST_CASE("image class")
{
    SECTION("test gray16")
    {
        mapnik::image_gray16 const im(4, 4);
        mapnik::image_gray16 im2(im);
        mapnik::image_gray16 im3(5, 5);

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

        CHECK(im(0, 0) == 0);
        CHECK(im2(0, 0) == 0);
        im2(0, 0) = 1;
        CHECK(im2(0, 0) == 1);
        im2.set(514);
        CHECK(im2(0, 0) == 514);
        CHECK(im2(1, 1) == 514);

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
            pixel_type const* data_1 = im.get_row(y);
            pixel_type const* data_2 = im.get_row(y, 1);
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
        pixel_type* data_im2 = im2.data();
        CHECK(*data_im2 == expected_val);
        unsigned char* data_b2 = im2.bytes();
        CHECK(*data_b2 == 2);
        ++data_b;
        CHECK(*data_b2 == 2);
        for (std::size_t y = 0; y < im2.height(); ++y)
        {
            std::size_t width = im2.width();
            pixel_type const* data_1 = im2.get_row(y);
            pixel_type const* data_2 = im2.get_row(y, 1);
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
        std::vector<pixel_type> v2(im2.width() - 1, 50);
        im2.set_row(0, v1.data(), v1.size());
        im2.set_row(1, 1, v2.size(), v2.data());

        CHECK(im2(0, 0) == 30);
        CHECK(im2(0, 1) == 514);
        CHECK(im2(1, 1) == 50);

    } // END SECTION

    SECTION("image_null")
    {
        mapnik::image_null im_null;
        mapnik::image_null const im_null2(2, 2); // Actually doesn't really set any size
        mapnik::image_null im_null3(im_null2);
        mapnik::image_null im_null4(std::move(im_null3));

        // All nulls are equal
        CHECK(im_null == im_null4);
        CHECK(im_null == im_null2);

        // No null is greater
        CHECK_FALSE(im_null < im_null4);
        CHECK_FALSE(im_null < im_null2);

        // Check defaults
        CHECK(im_null.width() == 0);
        CHECK(im_null.height() == 0);
        CHECK(im_null.size() == 0);
        CHECK(im_null.row_size() == 0);
        // Setting offset does nothing
        im_null.set_offset(10000000.0);
        CHECK(im_null.get_offset() == 0.0);
        // Setting scaling does nothing
        im_null.set_scaling(123123123.0);
        CHECK(im_null.get_scaling() == 1.0);
        CHECK(im_null.get_dtype() == mapnik::image_dtype_null);
        // Setting premultiplied does nothing
        im_null.set_premultiplied(true);
        CHECK_FALSE(im_null.get_premultiplied());
        // Setting painted does nothing
        im_null.painted(true);
        CHECK_FALSE(im_null.painted());

        // Should throw if we try to access or setdata.
        REQUIRE_THROWS(im_null(0, 0));
        REQUIRE_THROWS(im_null2(0, 0));
        REQUIRE_THROWS(im_null(0, 0) = 1);

        unsigned char const* e1 = im_null.bytes();
        unsigned char* e2 = im_null.bytes();
        CHECK(e1 == nullptr);
        CHECK(e2 == nullptr);

    } // END SECTION

    SECTION("image any")
    {
        mapnik::image_null null_im;
        mapnik::image_any const im_any_null(null_im);
        CHECK(im_any_null.get_dtype() == mapnik::image_dtype_null);
        CHECK(im_any_null.bytes() == nullptr);

        mapnik::image_gray16 im(4, 4);
        mapnik::fill(im, 514);
        mapnik::image_any im_any(im);

        CHECK(im_any.get_dtype() == mapnik::image_dtype_gray16);
        unsigned char* foo = im_any.bytes();
        CHECK(*foo == 2);
        ++foo;
        CHECK(*foo == 2);
        CHECK(im_any.width() == 4);
        CHECK(im_any.height() == 4);
        CHECK(im_any.size() == 32);
        CHECK(im_any.row_size() == 8);
        CHECK_FALSE(im_any.get_premultiplied());
        im_any.set_offset(10.0);
        CHECK(im_any.get_offset() == 10.0);
        im_any.set_scaling(2.1);
        CHECK(im_any.get_scaling() == 2.1);
        CHECK_FALSE(im_any.painted());

    } // END SECTION

    SECTION("test image_any initialization")
    {
        {
            mapnik::image_any im(4, 4);
            CHECK(im.get_dtype() == mapnik::image_dtype_rgba8);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_null);
            CHECK(im.get_dtype() == mapnik::image_dtype_null);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray8);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray8);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray8s);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray8s);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray16);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray16);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray16s);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray16s);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray32);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray32);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray32s);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray32s);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray32f);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray32f);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray64);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray64);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray64s);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray64s);
        }
        {
            mapnik::image_any im(4, 4, mapnik::image_dtype_gray64f);
            CHECK(im.get_dtype() == mapnik::image_dtype_gray64f);
        }

    } // END SECTION

    SECTION("Image Buffer")
    {
        mapnik::detail::buffer buf_zero(0);
        CHECK(buf_zero.size() == 0);
        CHECK(!buf_zero);
        mapnik::detail::buffer buf(10);
        CHECK(buf.size() == 10);
        CHECK_FALSE(!buf);
        unsigned char* d = buf.data();
        *d = 9;
        mapnik::detail::buffer const buf2 = buf;
        CHECK(buf2.size() == 10);
        unsigned char const* d2 = buf2.data();
        CHECK(*d2 == 9);

    } // END SECTION

    SECTION("Image copy/move")
    {
        mapnik::detail::buffer buf(16 * 16 * 4); // large enough to hold 16*16 RGBA image
        CHECK(buf.size() == 16 * 16 * 4);
        // fill buffer with 0xff
        std::fill(buf.data(), buf.data() + buf.size(), 0xff);

        // move buffer
        mapnik::detail::buffer buf2(std::move(buf));
        CHECK(buf.size() == 0);
        CHECK(buf.data() == nullptr);

        mapnik::image_rgba8 im(16, 16, buf2.data()); // shallow copy
        std::size_t count = 0;
        for (auto const& pixel : im)
        {
            // expect rgba(255,255,255,1.0)
            CHECK(sizeof(pixel) == sizeof(mapnik::image_rgba8::pixel_type));
            CHECK(pixel == 0xffffffff);
            ++count;
        }
        CHECK(count == im.width() * im.height());
        CHECK(buf2.size() == im.width() * im.height() * sizeof(mapnik::image_rgba8::pixel_type));

        // mutate buffer
        // fill buffer with 0x7f - semi-transparent grey
        std::fill(buf2.data(), buf2.data() + buf2.size(), 0x7f);
        for (auto const& pixel : im)
        {
            // expect rgba(127,127,127,0.5)
            CHECK(pixel == 0x7f7f7f7f);
        }

        // mutate image directly (buf)
        for (auto& pixel : im)
        {
            pixel = mapnik::color(0, 255, 0).rgba(); // green
        }

        // move
        mapnik::image_rgba8 im2(std::move(im));
        CHECK(im.data() == nullptr);
        CHECK(im.bytes() == nullptr);
        CHECK(im.width() == 0);
        CHECK(im.height() == 0);
        for (auto const& pixel : im2)
        {
            // expect `green`
            CHECK(pixel == mapnik::color(0, 255, 0).rgba());
        }

        mapnik::image_rgba8 im3(im2); // shallow copy
        for (auto& pixel : im3)
        {
            // expect `green`
            CHECK(pixel == mapnik::color(0, 255, 0).rgba());
            // mutate
            pixel = mapnik::color(255, 0, 0).rgba(); // red
        }

        for (auto const& pixel : im3)
        {
            // expect `red`
            CHECK(pixel == mapnik::color(255, 0, 0).rgba());
        }
        for (auto const& pixel : im2)
        {
            // expect `red`
            CHECK(pixel == mapnik::color(255, 0, 0).rgba());
        }
    }

    SECTION("image::swap")
    {
        auto blue = mapnik::color(50, 50, 250).rgba();
        auto orange = mapnik::color(250, 150, 0).rgba();

        mapnik::image_rgba8 im;
        mapnik::image_rgba8 im2(16, 16);
        mapnik::image_rgba8 im3(16, 16);

        im2.set(blue);
        im3.set(orange);

        // swap two non-empty images
        CHECK_NOTHROW(im2.swap(im3));
        CHECK(im2(0, 0) == orange);
        CHECK(im3(0, 0) == blue);

        // swap empty <-> non-empty
        CHECK_NOTHROW(im.swap(im3));
        CHECK(im3.data() == nullptr);
        CHECKED_IF(im.data() != nullptr)
        {
            CHECK(im(0, 0) == blue);
        }
    }

} // END TEST CASE
