#include "catch.hpp"

#include <mapnik/cairo_io.hpp>
#include <mapnik/filesystem.hpp>
#include <mapnik/util/fs.hpp>
#include <fstream>

#if defined(HAVE_CAIRO)
#include <cairo-version.h>

// see https://gitlab.freedesktop.org/cairo/cairo/-/issues/553
// TLDR: cairo has removed the writing of the svg version in cairo 1.17.6
#if (CAIRO_VERSION_MAJOR <= 1) && (CAIRO_VERSION_MINOR <= 17) && (CAIRO_VERSION_MICRO < 6)
TEST_CASE("cairo_io")
{
    SECTION("save_to_cairo_file - SVG")
    {
        std::string directory_name("/tmp/mapnik-tests/");
        mapnik::fs::create_directories(directory_name);
        REQUIRE(mapnik::util::exists(directory_name));

        std::string output_file(directory_name + "test_save_to_cairo_file.svg");

        mapnik::Map map(256, 256);
        mapnik::save_to_cairo_file(map, output_file);

        std::ifstream stream(output_file, std::ios_base::in | std::ios_base::binary);
        std::string actual_output(std::istreambuf_iterator<char>(stream.rdbuf()), std::istreambuf_iterator<char>());

        // Check the Cairo SVG surface is using SVG 1.2
        CHECK(actual_output.find("version=\"1.2\"") != std::string::npos);
    }
}
#endif
#endif
