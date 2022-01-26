// disabled on windows due to https://github.com/mapnik/mapnik/issues/2838
// TODO - get to the bottom of why including `tiff_reader.cpp` breaks windows
// or re-write image_readers to allow `#include tiff_reader.hpp`
#if !defined(_MSC_VER) && defined(HAVE_TIFF)

#include "catch.hpp"

#include <mapnik/color.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/util/fs.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include "../../../src/tiff_reader.cpp"

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
using source_type = boost::interprocess::ibufferstream;
#else
using source_type = std::filebuf;
#endif

#define TIFF_ASSERT(filename)                                                                                          \
    mapnik::tiff_reader<source_type> tiff_reader(filename);                                                            \
    REQUIRE(tiff_reader.width() == 256);                                                                               \
    REQUIRE(tiff_reader.height() == 256);                                                                              \
    REQUIRE(tiff_reader.planar_config() == PLANARCONFIG_CONTIG);                                                       \
    std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename, "tiff"));                          \
    REQUIRE(reader->width() == 256);                                                                                   \
    REQUIRE(reader->height() == 256);                                                                                  \
    mapnik::util::file file(filename);                                                                                 \
    mapnik::tiff_reader<mapnik::util::char_array_buffer> tiff_reader2(file.data().get(), file.size());                 \
    REQUIRE(tiff_reader2.width() == 256);                                                                              \
    REQUIRE(tiff_reader2.height() == 256);                                                                             \
    std::unique_ptr<mapnik::image_reader> reader2(mapnik::get_image_reader(file.data().get(), file.size()));           \
    REQUIRE(reader2->width() == 256);                                                                                  \
    REQUIRE(reader2->height() == 256);

#define TIFF_ASSERT_ALPHA(data)                                                                                        \
    REQUIRE(tiff_reader.has_alpha() == true);                                                                          \
    REQUIRE(reader->has_alpha() == true);                                                                              \
    REQUIRE(tiff_reader2.has_alpha() == true);                                                                         \
    REQUIRE(reader2->has_alpha() == true);                                                                             \
    REQUIRE(data.get_premultiplied() == true);

#define TIFF_ASSERT_NO_ALPHA_RGB(data)                                                                                 \
    REQUIRE(tiff_reader.has_alpha() == false);                                                                         \
    REQUIRE(reader->has_alpha() == false);                                                                             \
    REQUIRE(tiff_reader2.has_alpha() == false);                                                                        \
    REQUIRE(reader2->has_alpha() == false);                                                                            \
    REQUIRE(data.get_premultiplied() == true);

#define TIFF_ASSERT_NO_ALPHA_GRAY(data)                                                                                \
    REQUIRE(tiff_reader.has_alpha() == false);                                                                         \
    REQUIRE(reader->has_alpha() == false);                                                                             \
    REQUIRE(tiff_reader2.has_alpha() == false);                                                                        \
    REQUIRE(reader2->has_alpha() == false);                                                                            \
    REQUIRE(data.get_premultiplied() == false);

#define TIFF_ASSERT_SIZE(data, reader)                                                                                 \
    REQUIRE(data.width() == reader->width());                                                                          \
    REQUIRE(data.height() == reader->height());

#define TIFF_READ_ONE_PIXEL                                                                                            \
    mapnik::image_any subimage = reader->read(1, 1, 1, 1);                                                             \
    REQUIRE(subimage.width() == 1);                                                                                    \
    REQUIRE(subimage.height() == 1);

namespace {

template<typename Image>
struct test_image_traits
{
    using value_type = mapnik::color;
    static value_type const& get_value(mapnik::color const& c) { return c; }
};

template<>
struct test_image_traits<mapnik::image_gray8>
{
    using value_type = std::uint8_t;
    static value_type get_value(mapnik::color const& c)
    {
        return c.green(); // use green channel for gray scale images
    }
};

template<typename Image>
Image generate_test_image()
{
    std::size_t tile_size = 16;
    Image im(64, 64);
    mapnik::color colors[] = {{mapnik::color("red")},
                              {mapnik::color("blue")},
                              {mapnik::color("green")},
                              {mapnik::color("yellow")}};
    std::size_t index = 0;
    for (std::size_t j = 0; j < im.height() / tile_size; ++j)
    {
        ++index;
        for (std::size_t i = 0; i < im.width() / tile_size; ++i)
        {
            ++index;
            for (std::size_t x = 0; x < tile_size; ++x)
            {
                for (std::size_t y = 0; y < tile_size; ++y)
                {
                    auto value = test_image_traits<Image>::get_value(colors[index % 4]);
                    mapnik::set_pixel(im, i * tile_size + x, j * tile_size + y, value);
                }
            }
        }
    }
    return im;
}

template<typename Image1, typename Image2>
bool identical(Image1 const& im1, Image2 const& im2)
{
    if ((im1.width() != im2.width()) || (im1.height() != im2.height()))
        return false;

    for (std::size_t i = 0; i < im1.width(); ++i)
    {
        for (std::size_t j = 0; j < im1.height(); ++j)
        {
            if (im1(i, j) != im2(i, j))
                return false;
        }
    }
    return true;
}

template<typename Image>
void test_tiff_reader(std::string const& pattern)
{
    // generate expected image (rgba8 or gray8)
    auto im = generate_test_image<Image>();

    for (auto const& path : mapnik::util::list_directory("test/data/tiff/"))
    {
        if (boost::iends_with(path, ".tif") && boost::istarts_with(mapnik::util::basename(path), pattern))
        {
            mapnik::tiff_reader<source_type> tiff_reader(path);
            auto width = tiff_reader.width();
            auto height = tiff_reader.height();
            {
                // whole image
                auto tiff = tiff_reader.read(0, 0, width, height);
                CHECK(tiff.is<Image>());
                auto im2 = tiff.get<Image>();
                REQUIRE(identical(im, im2));
            }
            {
                // portion
                auto tiff = tiff_reader.read(11, 13, width - 11, height - 13);
                CHECK(tiff.is<Image>());
                auto im2 = tiff.get<Image>();
                auto view = mapnik::image_view<Image>(11, 13, width, height, im);
                REQUIRE(identical(view, im2));
            }
        }
    }
}

} // namespace

TEST_CASE("tiff io")
{
    SECTION("tiff-reader rgb8+rgba8") { test_tiff_reader<mapnik::image_rgba8>("tiff_rgb"); }

    SECTION("tiff-reader gray8") { test_tiff_reader<mapnik::image_gray8>("tiff_gray"); }

    SECTION("scan rgb8 striped")
    {
        std::string filename("./test/data/tiff/scan_512x512_rgb8_striped.tif");
        mapnik::tiff_reader<source_type> tiff_reader(filename);
        REQUIRE(tiff_reader.width() == 512);
        REQUIRE(tiff_reader.height() == 512);
        REQUIRE(tiff_reader.planar_config() == PLANARCONFIG_CONTIG);
        REQUIRE(tiff_reader.rows_per_strip() == 16);
        REQUIRE(tiff_reader.bits_per_sample() == 8);
        REQUIRE(tiff_reader.is_tiled() == false);
        REQUIRE(tiff_reader.tile_width() == 0);
        REQUIRE(tiff_reader.tile_height() == 0);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_PALETTE);
        REQUIRE(tiff_reader.compression() == COMPRESSION_NONE);
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename, "tiff"));
        REQUIRE(reader->width() == 512);
        REQUIRE(reader->height() == 512);
        mapnik::util::file file(filename);
        mapnik::tiff_reader<mapnik::util::char_array_buffer> tiff_reader2(file.data().get(), file.size());
        REQUIRE(tiff_reader2.width() == 512);
        REQUIRE(tiff_reader2.height() == 512);
        std::unique_ptr<mapnik::image_reader> reader2(mapnik::get_image_reader(file.data().get(), file.size()));
        REQUIRE(reader2->width() == 512);
        REQUIRE(reader2->height() == 512);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_rgba8>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_RGB(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("scan rgb8 tiled")
    {
        std::string filename("./test/data/tiff/scan_512x512_rgb8_tiled.tif");
        mapnik::tiff_reader<source_type> tiff_reader(filename);
        REQUIRE(tiff_reader.width() == 512);
        REQUIRE(tiff_reader.height() == 512);
        REQUIRE(tiff_reader.planar_config() == PLANARCONFIG_CONTIG);
        REQUIRE(tiff_reader.rows_per_strip() == 0);
        REQUIRE(tiff_reader.bits_per_sample() == 8);
        REQUIRE(tiff_reader.is_tiled() == true);
        REQUIRE(tiff_reader.tile_width() == 256);
        REQUIRE(tiff_reader.tile_height() == 256);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_PALETTE);
        REQUIRE(tiff_reader.compression() == COMPRESSION_LZW);
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename, "tiff"));
        REQUIRE(reader->width() == 512);
        REQUIRE(reader->height() == 512);
        mapnik::util::file file(filename);
        mapnik::tiff_reader<mapnik::util::char_array_buffer> tiff_reader2(file.data().get(), file.size());
        REQUIRE(tiff_reader2.width() == 512);
        REQUIRE(tiff_reader2.height() == 512);
        std::unique_ptr<mapnik::image_reader> reader2(mapnik::get_image_reader(file.data().get(), file.size()));
        REQUIRE(reader2->width() == 512);
        REQUIRE(reader2->height() == 512);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_rgba8>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_RGB(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("rgba8 striped")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_rgba8_striped.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 1);
        REQUIRE(tiff_reader.bits_per_sample() == 8);
        REQUIRE(tiff_reader.is_tiled() == false);
        REQUIRE(tiff_reader.tile_width() == 0);
        REQUIRE(tiff_reader.tile_height() == 0);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_RGB);
        REQUIRE(tiff_reader.compression() == COMPRESSION_ADOBE_DEFLATE);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_rgba8>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_ALPHA(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("rgba8 tiled")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_rgba8_tiled.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 0);
        REQUIRE(tiff_reader.bits_per_sample() == 8);
        REQUIRE(tiff_reader.is_tiled() == true);
        REQUIRE(tiff_reader.tile_width() == 256);
        REQUIRE(tiff_reader.tile_height() == 256);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_RGB);
        REQUIRE(tiff_reader.compression() == COMPRESSION_LZW);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_rgba8>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_ALPHA(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("rgb8 striped")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_rgb8_striped.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 10);
        REQUIRE(tiff_reader.bits_per_sample() == 8);
        REQUIRE(tiff_reader.is_tiled() == false);
        REQUIRE(tiff_reader.tile_width() == 0);
        REQUIRE(tiff_reader.tile_height() == 0);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_RGB);
        REQUIRE(tiff_reader.compression() == COMPRESSION_NONE);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_rgba8>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_RGB(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("rgb8 tiled")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_rgb8_tiled.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 0);
        REQUIRE(tiff_reader.bits_per_sample() == 8);
        REQUIRE(tiff_reader.is_tiled() == true);
        REQUIRE(tiff_reader.tile_width() == 32);
        REQUIRE(tiff_reader.tile_height() == 32);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_RGB);
        REQUIRE(tiff_reader.compression() == COMPRESSION_LZW);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_rgba8>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_RGB(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("gray8 striped")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_gray8_striped.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 32);
        REQUIRE(tiff_reader.bits_per_sample() == 8);
        REQUIRE(tiff_reader.is_tiled() == false);
        REQUIRE(tiff_reader.tile_width() == 0);
        REQUIRE(tiff_reader.tile_height() == 0);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_MINISBLACK);
        REQUIRE(tiff_reader.compression() == COMPRESSION_NONE);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_gray8>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_GRAY(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("gray8 tiled")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_gray8_tiled.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 0);
        REQUIRE(tiff_reader.bits_per_sample() == 8);
        REQUIRE(tiff_reader.is_tiled() == true);
        REQUIRE(tiff_reader.tile_width() == 256);
        REQUIRE(tiff_reader.tile_height() == 256);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_MINISBLACK);
        REQUIRE(tiff_reader.compression() == COMPRESSION_LZW);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_gray8>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_GRAY(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("gray16 striped")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_gray16_striped.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 16);
        REQUIRE(tiff_reader.bits_per_sample() == 16);
        REQUIRE(tiff_reader.is_tiled() == false);
        REQUIRE(tiff_reader.tile_width() == 0);
        REQUIRE(tiff_reader.tile_height() == 0);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_MINISBLACK);
        REQUIRE(tiff_reader.compression() == COMPRESSION_NONE);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_gray16>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_GRAY(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("gray16 tiled")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_gray16_tiled.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 0);
        REQUIRE(tiff_reader.bits_per_sample() == 16);
        REQUIRE(tiff_reader.is_tiled() == true);
        REQUIRE(tiff_reader.tile_width() == 256);
        REQUIRE(tiff_reader.tile_height() == 256);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_MINISBLACK);
        REQUIRE(tiff_reader.compression() == COMPRESSION_LZW);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_gray16>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_GRAY(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("gray32f striped")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_gray32f_striped.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 8);
        REQUIRE(tiff_reader.bits_per_sample() == 32);
        REQUIRE(tiff_reader.is_tiled() == false);
        REQUIRE(tiff_reader.tile_width() == 0);
        REQUIRE(tiff_reader.tile_height() == 0);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_MINISBLACK);
        REQUIRE(tiff_reader.compression() == COMPRESSION_NONE);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_gray32f>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_GRAY(data);
        TIFF_READ_ONE_PIXEL
    }

    SECTION("gray32f tiled")
    {
        TIFF_ASSERT("./test/data/tiff/ndvi_256x256_gray32f_tiled.tif")
        REQUIRE(tiff_reader.rows_per_strip() == 0);
        REQUIRE(tiff_reader.bits_per_sample() == 32);
        REQUIRE(tiff_reader.is_tiled() == true);
        REQUIRE(tiff_reader.tile_width() == 256);
        REQUIRE(tiff_reader.tile_height() == 256);
        REQUIRE(tiff_reader.photometric() == PHOTOMETRIC_MINISBLACK);
        REQUIRE(tiff_reader.compression() == COMPRESSION_LZW);
        mapnik::image_any data = reader->read(0, 0, reader->width(), reader->height());
        REQUIRE(data.is<mapnik::image_gray32f>() == true);
        TIFF_ASSERT_SIZE(data, reader);
        TIFF_ASSERT_NO_ALPHA_GRAY(data);
        TIFF_READ_ONE_PIXEL
    }
}

#endif
