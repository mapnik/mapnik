#include "catch.hpp"

#include <mapnik/geometry/closest_point.hpp>

TEST_CASE("geometry closest point") {

#if BOOST_VERSION >= 106200

SECTION("geometry_empty") {

    mapnik::geometry::point<double> pt(0, 0);
    mapnik::geometry::geometry_empty empty;
    auto result = mapnik::geometry::closest_point(empty, pt);
    REQUIRE(result.x == 0.0);
    REQUIRE(result.y == 0.0);
    REQUIRE(result.distance == -1.0);
}

SECTION("point") {

    mapnik::geometry::point<double> pt(0, 0);
    mapnik::geometry::point<double> geom(3.0, 4.0);
    auto result = mapnik::geometry::closest_point(geom, pt);
    REQUIRE(result.x == geom.x);
    REQUIRE(result.y == geom.y);
    REQUIRE(result.distance == 5.0);
}

SECTION("linestring") {

    mapnik::geometry::line_string<double> line;
    line.emplace_back(0, 0);
    line.emplace_back(0, 100);
    line.emplace_back(100, 100);
    line.emplace_back(100, 0);
    mapnik::geometry::point<double> pt(50, 50);
    auto result = mapnik::geometry::closest_point(line, pt);
    REQUIRE(result.x == 0.0);
    REQUIRE(result.y == 50.0);
    REQUIRE(result.distance == 50.0);
}


SECTION("polygon") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0, 0);
    ring.emplace_back(1, 0);
    ring.emplace_back(1, 1);
    ring.emplace_back(0, 1);
    ring.emplace_back(0, 0);
    poly.push_back(std::move(ring));

    {
        // point inside polygon
        mapnik::geometry::point<double> pt {0.5, 0.25};
        auto result = mapnik::geometry::closest_point(poly, pt);
        REQUIRE(result.x == 0.5);
        REQUIRE(result.y == 0.25);
        REQUIRE(result.distance == 0.0);
    }
    {
        // point outside polygon
        mapnik::geometry::point<double> pt {1.25, 0.5};
        auto result = mapnik::geometry::closest_point(poly, pt);
        REQUIRE(result.x == 1.0);
        REQUIRE(result.y == 0.5);
        REQUIRE(result.distance == 0.25);
    }
    {
        // point outside polygon
        mapnik::geometry::point<double> pt {4.0, 5.0};
        auto result = mapnik::geometry::closest_point(poly, pt);
        REQUIRE(result.x == 1.0);
        REQUIRE(result.y == 1.0);
        REQUIRE(result.distance == 5.0);
    }
    {
        // point on polygon boundary
        mapnik::geometry::point<double> pt {0, 0.4};
        auto result = mapnik::geometry::closest_point(poly, pt);
        REQUIRE(result.x == 0.0);
        REQUIRE(result.y == 0.4);
        REQUIRE(result.distance == 0.0);
    }
}
#endif
}
