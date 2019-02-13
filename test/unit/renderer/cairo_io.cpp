#include "catch.hpp"

#include <mapnik/cairo_io.hpp>
#include <mapnik/util/fs.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/filesystem/convenience.hpp>
#pragma GCC diagnostic pop

#include <fstream>

#if defined(HAVE_CAIRO)
TEST_CASE("cairo_io") {

SECTION("save_to_cairo_file - SVG") {
    std::string directory_name("/tmp/mapnik-tests/");
    boost::filesystem::create_directories(directory_name);
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
