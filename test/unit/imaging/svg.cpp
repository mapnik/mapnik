#include "catch.hpp"

// mapnik
#include <mapnik/image_any.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_view_any.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/image_any.hpp>

#include <iostream>

TEST_CASE("image class svg features") {

SECTION("test magic number search") {
    
} // END SECTION

SECTION("svg_blank")
{
    std::string imagedata = "";
    CHECK_THROWS(std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(imagedata.c_str(), imagedata.length())));
}

SECTION("svg_invalid")
{
    std::string imagedata = "<svg/svg>";
    std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(imagedata.c_str(), imagedata.length()));
    CHECK(reader.get());
    unsigned width = reader->width();
    unsigned height = reader->height();
  
    CHECK(width == 0);
    CHECK(height == 0);
}

SECTION("svg_empty")
{
    std::string imagedata = "<svg></svg>";
    std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(imagedata.c_str(), imagedata.length()));
    CHECK(reader.get());
    unsigned width = reader->width();
    unsigned height = reader->height();
  
    CHECK(width == 0);
    CHECK(height == 0);
}

SECTION("svg_blank")
{
    std::string imagedata = "<svg width='100' height='100'><g id='a'><ellipse fill='#FFFFFF' stroke='#000000' stroke-width='4' cx='50' cy='50' rx='25' ry='25'/></g></svg>";
    std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(imagedata.c_str(), imagedata.length()));
    CHECK(reader.get());
    unsigned width = reader->width();
    unsigned height = reader->height();
  
    CHECK(width == 100);
    CHECK(height == 100);
  
    mapnik::image_any im = reader->read(0,0,width,height);
  
    mapnik::image_rgba8 raw = im.get<mapnik::image_rgba8>();

    std::string pngdata = mapnik::save_to_string<mapnik::image_rgba8>(raw,"png");
    CHECK(pngdata.length() == 1270 );
  
} // END SECTION

} // END TEST CASE

