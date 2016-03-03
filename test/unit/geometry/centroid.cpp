
#include "catch.hpp"

#include <mapnik/geometry_centroid.hpp>

TEST_CASE("geometry centroid") {

SECTION("empty geometry") {

    mapnik::geometry::geometry_empty geom;
    mapnik::geometry::point<double> centroid;
    REQUIRE(!mapnik::geometry::centroid(geom, centroid));
}

SECTION("geometry collection") {

    mapnik::geometry::geometry_collection<double> geom;
    mapnik::geometry::point<double> centroid;
    REQUIRE(!mapnik::geometry::centroid(geom, centroid));
}

SECTION("point") {

    mapnik::geometry::point<double> pt(10, 10);
    mapnik::geometry::point<double> centroid;
    REQUIRE(mapnik::geometry::centroid(pt, centroid));
    REQUIRE(pt.x == centroid.x);
    REQUIRE(pt.y == centroid.y);
}

SECTION("linestring") {

    mapnik::geometry::line_string<double> line;
    line.add_coord(0, 0);
    line.add_coord(25, 25);
    line.add_coord(50, 50);
    mapnik::geometry::point<double> centroid;
    REQUIRE(mapnik::geometry::centroid(line, centroid));
    REQUIRE(centroid.x == 25);
    REQUIRE(centroid.y == 25);
}

SECTION("empty linestring") {

    mapnik::geometry::line_string<double> line;
    mapnik::geometry::point<double> centroid;
    REQUIRE(!mapnik::geometry::centroid(line, centroid));
}

SECTION("polygon") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.add_coord(0, 0);
    ring.add_coord(1, 0);
    ring.add_coord(1, 1);
    ring.add_coord(0, 1);
    ring.add_coord(0, 0);
    poly.set_exterior_ring(std::move(ring));

    mapnik::geometry::point<double> centroid;
    REQUIRE(mapnik::geometry::centroid(poly, centroid));
    REQUIRE(centroid.x == 0.5);
    REQUIRE(centroid.y == 0.5);
}

SECTION("polygon with empty exterior ring") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    poly.set_exterior_ring(std::move(ring));

    mapnik::geometry::point<double> centroid;
    REQUIRE(!mapnik::geometry::centroid(poly, centroid));
}

SECTION("empty polygon") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::point<double> centroid;
    REQUIRE(!mapnik::geometry::centroid(poly, centroid));
}

SECTION("multi-point") {

    mapnik::geometry::multi_point<double> geom;
    geom.add_coord(0, 0);
    geom.add_coord(25, 25);
    geom.add_coord(50, 50);
    mapnik::geometry::point<double> centroid;
    REQUIRE(mapnik::geometry::centroid(geom, centroid));
    REQUIRE(centroid.x == 25);
    REQUIRE(centroid.y == 25);
}

SECTION("empty multi-point") {

    mapnik::geometry::multi_point<double> geom;
    mapnik::geometry::point<double> centroid;
    REQUIRE(!mapnik::geometry::centroid(geom, centroid));
}

SECTION("multi-linestring") {

    mapnik::geometry::multi_line_string<double> geom;
    {
        mapnik::geometry::line_string<double> line;
        line.add_coord(0, 0);
        line.add_coord(0, 25);
        line.add_coord(0, 50);
        geom.emplace_back(std::move(line));
    }
    {
        mapnik::geometry::line_string<double> line;
        line.add_coord(0, 0);
        line.add_coord(25, 0);
        line.add_coord(50, 0);
        geom.emplace_back(std::move(line));
    }
    mapnik::geometry::point<double> centroid;
    REQUIRE(mapnik::geometry::centroid(geom, centroid));
    REQUIRE(centroid.x == 12.5);
    REQUIRE(centroid.y == 12.5);
}

SECTION("multi-linestring: one component empty") {

    mapnik::geometry::multi_line_string<double> geom;
    mapnik::geometry::line_string<double> line;
    line.add_coord(0, 0);
    line.add_coord(0, 25);
    line.add_coord(0, 50);
    geom.emplace_back(std::move(line));
    geom.emplace_back();
    mapnik::geometry::point<double> centroid;
    REQUIRE(mapnik::geometry::centroid(geom, centroid));
    REQUIRE(centroid.x == 0);
    REQUIRE(centroid.y == 25);
}

SECTION("empty multi-linestring") {

    mapnik::geometry::multi_line_string<double> geom;
    mapnik::geometry::point<double> centroid;
    REQUIRE(!mapnik::geometry::centroid(geom, centroid));
}

SECTION("multi-polygon") {

    mapnik::geometry::multi_polygon<double> geom;
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.add_coord(0, 0);
        ring.add_coord(1, 0);
        ring.add_coord(1, 1);
        ring.add_coord(0, 1);
        ring.add_coord(0, 0);
        poly.set_exterior_ring(std::move(ring));
        geom.emplace_back(std::move(poly));
    }
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.add_coord(1, 1);
        ring.add_coord(2, 1);
        ring.add_coord(2, 2);
        ring.add_coord(1, 2);
        ring.add_coord(1, 1);
        poly.set_exterior_ring(std::move(ring));
        geom.emplace_back(std::move(poly));
    }

    mapnik::geometry::point<double> centroid;
    REQUIRE(mapnik::geometry::centroid(geom, centroid));
    REQUIRE(centroid.x == 1);
    REQUIRE(centroid.y == 1);
}

SECTION("multi-polygon: one component empty") {

    mapnik::geometry::multi_polygon<double> geom;
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

    mapnik::geometry::point<double> centroid;
    REQUIRE(mapnik::geometry::centroid(geom, centroid));
    REQUIRE(centroid.x == 0.5);
    REQUIRE(centroid.y == 0.5);
}

SECTION("empty multi-polygon") {

    mapnik::geometry::multi_polygon<double> geom;
    mapnik::geometry::point<double> centroid;
    REQUIRE(!mapnik::geometry::centroid(geom, centroid));
}
}
