#include "catch.hpp"
#include "fake_path.hpp"

// mapnik
#include <mapnik/extend_converter.hpp>

// stl
#include <iostream>

namespace offset_test {

TEST_CASE("extend converter")
{
    SECTION("empty")
    {
        try
        {
            fake_path path = {};
            mapnik::extend_converter<fake_path> c(path, 1000);
            double x, y;
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("one point")
    {
        try
        {
            fake_path path = {0, 0};
            mapnik::extend_converter<fake_path> c(path, 1000);
            double x, y;
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_MOVETO);
            REQUIRE(x == 0);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("two points")
    {
        try
        {
            fake_path path = {0, 0, 1, 0};
            mapnik::extend_converter<fake_path> c(path, 1000);
            double x, y;
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_MOVETO);
            REQUIRE(x == -1000);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
            REQUIRE(x == 1001);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("three points")
    {
        try
        {
            fake_path path = {0, 0, 1, 0, 2, 0};
            mapnik::extend_converter<fake_path> c(path, 1000);
            double x, y;
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_MOVETO);
            REQUIRE(x == -1000);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
            REQUIRE(x == 1);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
            REQUIRE(x == 1002);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }

    SECTION("more points")
    {
        try
        {
            fake_path path = {0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0};
            mapnik::extend_converter<fake_path> c(path, 1000);
            double x, y;
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_MOVETO);
            REQUIRE(x == -1000);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
            REQUIRE(x == 1);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
            REQUIRE(x == 2);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
            REQUIRE(x == 3);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
            REQUIRE(x == 4);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_LINETO);
            REQUIRE(x == 1005);
            REQUIRE(y == 0);
            REQUIRE(c.vertex(&x, &y) == mapnik::SEG_END);
        }
        catch (std::exception const& ex)
        {
            std::cerr << ex.what() << "\n";
            REQUIRE(false);
        }
    }
}

} // namespace offset_test
