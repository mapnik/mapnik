#include "catch.hpp"

#include <mapnik/geometry/remove_empty.hpp>

TEST_CASE("geometry remove_empty")
{
    SECTION("point")
    {
        using geom_type = mapnik::geometry::point<double>;
        geom_type pt(10, 10);
        geom_type pt2 = mapnik::geometry::remove_empty(pt);
        REQUIRE(pt.x == pt2.x);
        REQUIRE(pt.y == pt2.y);
    }

    SECTION("multi-linestring")
    {
        using geom_type = mapnik::geometry::multi_line_string<double>;
        geom_type geom;
        mapnik::geometry::line_string<double> line;
        line.emplace_back(0, 0);
        line.emplace_back(0, 25);
        line.emplace_back(0, 50);
        geom.emplace_back(std::move(line));
        geom.emplace_back();

        REQUIRE(geom.size() == 2);
        geom_type geom2 = mapnik::geometry::remove_empty(geom);
        REQUIRE(geom2.size() == 1);
        REQUIRE(geom2[0].size() == 3);
    }

    SECTION("multi-polygon")
    {
        using geom_type = mapnik::geometry::multi_polygon<double>;
        geom_type geom;
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(1, 0);
        ring.emplace_back(1, 1);
        ring.emplace_back(0, 1);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        geom.emplace_back(std::move(poly));
        geom.emplace_back();
        // geom.back().emplace_back(); //add an empty exterior ring
        REQUIRE(geom.size() == 2);
        geom_type geom2 = mapnik::geometry::remove_empty(geom);
        REQUIRE(geom2.size() == 1);
        REQUIRE(geom2.front().front().size() == 5);
    }
}
