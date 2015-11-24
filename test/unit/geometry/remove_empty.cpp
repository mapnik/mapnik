#include "catch.hpp"

#include <mapnik/geometry_remove_empty.hpp>

TEST_CASE("geometry remove_empty") {

SECTION("point") {

    using geom_type = mapnik::geometry::point<double>;
    geom_type pt(10, 10);
    geom_type pt2 = mapnik::geometry::remove_empty(pt);
    REQUIRE(pt.x == pt2.x);
    REQUIRE(pt.y == pt2.y);
}

SECTION("multi-linestring") {

    using geom_type = mapnik::geometry::multi_line_string<double>;
    geom_type geom;
    mapnik::geometry::line_string<double> line;
    line.add_coord(0, 0);
    line.add_coord(0, 25);
    line.add_coord(0, 50);
    geom.emplace_back(std::move(line));
    geom.emplace_back();

    REQUIRE(geom.size() == 2);
    geom_type geom2 = mapnik::geometry::remove_empty(geom);
    REQUIRE(geom2.size() == 1);
    REQUIRE(geom2[0].size() == 3);
}

SECTION("multi-polygon") {

    using geom_type = mapnik::geometry::multi_polygon<double>;
    geom_type geom;
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.add_coord(0, 0);
    ring.add_coord(1, 0);
    ring.add_coord(1, 1);
    ring.add_coord(0, 1);
    ring.add_coord(0, 0);
    poly.set_exterior_ring(std::move(ring));
    geom.emplace_back(std::move(poly));
    geom.emplace_back();

    REQUIRE(geom.size() == 2);
    geom_type geom2 = mapnik::geometry::remove_empty(geom);
    REQUIRE(geom2.size() == 1);
    REQUIRE(geom2[0].exterior_ring.size() == 5);
}
}
