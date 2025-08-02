// disabled on windows due to https://github.com/mapnik/mapnik/issues/2838
// TODO - get to the bottom of why including `tiff_reader.cpp` breaks windows
// or re-write image_readers to allow `#include tiff_reader.hpp`
#ifdef HAVE_AVIF

#include "catch.hpp"

#include <mapnik/color.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/util/fs.hpp>
#include <iostream>
namespace {

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
            {
                std::cerr << i << ":" << j << std::endl;
                return false;
            }
        }
    }
    return true;
}

} // namespace

TEST_CASE("avif io")
{
    SECTION("avif yuv:444 depth:8 lossless")
    {
        std::string filename("./test/data/images/12_654_1580-yuv444-d8-lossless.avif");
        std::string filename2("./test/data/images/12_654_1580.png");
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename));
        std::unique_ptr<mapnik::image_reader> reader2(mapnik::get_image_reader(filename2));
        REQUIRE(!reader->has_alpha());
        REQUIRE(reader->width() == 256);
        REQUIRE(reader->height() == 256);
        mapnik::image_rgba8 im1(256, 256);
        reader->read(0, 0, im1);
        mapnik::image_rgba8 im2(256, 256);
        reader2->read(0,0,im2);
        REQUIRE(identical(im1, im2));
        REQUIRE(im1.get_dtype() == mapnik::image_dtype::image_dtype_rgba8);
    }

    SECTION("avif yuv:444 depth:8")
    {
        std::string filename("./test/data/images/12_654_1580-yuv444-d8.avif");
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename));
        REQUIRE(reader->width() == 256);
        REQUIRE(reader->height() == 256);
        REQUIRE(!reader->has_alpha());
        auto image = reader->read(28, 28, 256 - 2*28, 256 - 2*28);
        REQUIRE(image.width() == 200);
        REQUIRE(image.height() == 200);
        REQUIRE(image.get_dtype() == mapnik::image_dtype::image_dtype_rgba8);
    }
    SECTION("avif yuv:400 depth:12")
    {
        std::string filename("./test/data/images/12_654_1580-yuv400-d12.avif");
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename));
        REQUIRE(reader->width() == 256);
        REQUIRE(reader->height() == 256);
        REQUIRE(!reader->has_alpha());
        auto image = reader->read(28, 28, 256 - 2*28, 256 - 2*28);
        REQUIRE(image.width() == 200);
        REQUIRE(image.height() == 200);
        REQUIRE(image.get_dtype() == mapnik::image_dtype::image_dtype_rgba8);
    }
    SECTION("avif yuv:420 depth:8")
    {
        std::string filename("./test/data/images/12_654_1580-yuv420-d8.avif");
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename));
        REQUIRE(reader->width() == 256);
        REQUIRE(reader->height() == 256);
        REQUIRE(!reader->has_alpha());
        auto image = reader->read(28, 28, 256 - 2*28, 256 - 2*28);
        REQUIRE(image.width() == 200);
        REQUIRE(image.height() == 200);
        REQUIRE(image.get_dtype() == mapnik::image_dtype::image_dtype_rgba8);
    }
    SECTION("avif yuv:422 depth:10")
    {
        std::string filename("./test/data/images/12_654_1580-yuv422-d10.avif");
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename));
        REQUIRE(reader->width() == 256);
        REQUIRE(reader->height() == 256);
        REQUIRE(!reader->has_alpha());
        auto image = reader->read(28, 28, 256 - 2*28, 256 - 2*28);
        REQUIRE(image.width() == 200);
        REQUIRE(image.height() == 200);
        REQUIRE(image.get_dtype() == mapnik::image_dtype::image_dtype_rgba8);
    }
    SECTION("avif yuv:422 depth:12")
    {
        std::string filename("./test/data/images/12_654_1580-yuv422-d12.avif");
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename));
        REQUIRE(reader->width() == 256);
        REQUIRE(reader->height() == 256);
        REQUIRE(!reader->has_alpha());
        auto image = reader->read(28, 28, 256 - 2*28, 256 - 2*28);
        REQUIRE(image.width() == 200);
        REQUIRE(image.height() == 200);
        REQUIRE(image.get_dtype() == mapnik::image_dtype::image_dtype_rgba8);
    }
    SECTION("avif yuv:444 depth:8")
    {
        std::string filename("./test/data/images/12_654_1580-yuv444-d8.avif");
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(filename));
        REQUIRE(reader->width() == 256);
        REQUIRE(reader->height() == 256);
        REQUIRE(!reader->has_alpha());
        auto image = reader->read(28, 28, 256 - 2*28, 256 - 2*28);
        REQUIRE(image.width() == 200);
        REQUIRE(image.height() == 200);
        REQUIRE(image.get_dtype() == mapnik::image_dtype::image_dtype_rgba8);
    }
}

#endif
