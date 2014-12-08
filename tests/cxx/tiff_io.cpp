#include "catch.hpp"

#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/tiff_io.hpp>
#include "../../src/tiff_reader.cpp"

std::unique_ptr<mapnik::image_reader> open(std::string const& filename)
{
    return std::unique_ptr<mapnik::image_reader>(mapnik::get_image_reader(filename,"tiff")); 
}

#define TIFF_ASSERT(filename) \
    mapnik::tiff_reader<boost::iostreams::file_source> tiff_reader(filename); \
    REQUIRE( tiff_reader.width() == 256 ); \
    REQUIRE( tiff_reader.height() == 256 ); \
    REQUIRE( tiff_reader.has_alpha() == false ); \
    REQUIRE( tiff_reader.premultiplied_alpha() == false ); \
    auto reader = open(filename); \
    REQUIRE( reader->width() == 256 ); \
    REQUIRE( reader->height() == 256 ); \
    REQUIRE( reader->has_alpha() == false ); \
    REQUIRE( reader->premultiplied_alpha() == false ); \

TEST_CASE("tiff io") {

SECTION("gray8 striped") {
    TIFF_ASSERT("./tests/data/tiff/ndvi_256x256_gray8_striped.tif")
    REQUIRE( tiff_reader.bits_per_sample() == 8 );
    REQUIRE( tiff_reader.is_tiled() == false );
    REQUIRE( tiff_reader.tile_width() == 0 );
    REQUIRE( tiff_reader.tile_height() == 0 );
}

SECTION("gray8 tiled") {
    TIFF_ASSERT("./tests/data/tiff/ndvi_256x256_gray8_tiled.tif")
    REQUIRE( tiff_reader.bits_per_sample() == 8 );
    REQUIRE( tiff_reader.is_tiled() == true );
    REQUIRE( tiff_reader.tile_width() == 256 );
    REQUIRE( tiff_reader.tile_height() == 256 );
}

SECTION("gray16 striped") {
    TIFF_ASSERT("./tests/data/tiff/ndvi_256x256_gray16_striped.tif")
    REQUIRE( tiff_reader.bits_per_sample() == 16 );
    REQUIRE( tiff_reader.is_tiled() == false );
    REQUIRE( tiff_reader.tile_width() == 0 );
    REQUIRE( tiff_reader.tile_height() == 0 );
}

SECTION("gray16 tiled") {
    TIFF_ASSERT("./tests/data/tiff/ndvi_256x256_gray16_tiled.tif")
    REQUIRE( tiff_reader.bits_per_sample() == 16 );
    REQUIRE( tiff_reader.is_tiled() == true );
    REQUIRE( tiff_reader.tile_width() == 256 );
    REQUIRE( tiff_reader.tile_height() == 256 );
}

SECTION("rgba8 striped") {
    TIFF_ASSERT("./tests/data/tiff/ndvi_256x256_rgba8_striped.tif")
    REQUIRE( tiff_reader.bits_per_sample() == 8 );
    REQUIRE( tiff_reader.is_tiled() == false );
    REQUIRE( tiff_reader.tile_width() == 0 );
    REQUIRE( tiff_reader.tile_height() == 0 );
}

SECTION("rgba8 tiled") {
    TIFF_ASSERT("./tests/data/tiff/ndvi_256x256_rgba8_tiled.tif")
    REQUIRE( tiff_reader.bits_per_sample() == 8 );
    REQUIRE( tiff_reader.is_tiled() == true );
    REQUIRE( tiff_reader.tile_width() == 256 );
    REQUIRE( tiff_reader.tile_height() == 256 );
}

SECTION("gray32f striped") {
    TIFF_ASSERT("./tests/data/tiff/ndvi_256x256_gray32f_striped.tif")
    REQUIRE( tiff_reader.bits_per_sample() == 32 );
    REQUIRE( tiff_reader.is_tiled() == false );
    REQUIRE( tiff_reader.tile_width() == 0 );
    REQUIRE( tiff_reader.tile_height() == 0 );
}

SECTION("gray32f tiled") {
    TIFF_ASSERT("./tests/data/tiff/ndvi_256x256_gray32f_tiled.tif")
    REQUIRE( tiff_reader.bits_per_sample() == 32 );
    REQUIRE( tiff_reader.is_tiled() == true );
    REQUIRE( tiff_reader.tile_width() == 256 );
    REQUIRE( tiff_reader.tile_height() == 256 );
}

}