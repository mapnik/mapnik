#include "catch.hpp"

#include <mapnik/geometry/closest_point.hpp>

TEST_CASE("geometry closest point") {

SECTION("point") {

    mapnik::geometry::point<double> pt(0, 0);
    mapnik::geometry::point<double> geom(3.0, 4.0);
    auto result = mapnik::geometry::closest_point(pt, geom);
    REQUIRE(result.closest_point == geom);
    REQUIRE(result.distance == 5.0);
}

SECTION("linestring") {

    //mapnik::geometry::line_string<double> line;
    //line.emplace_back(0, 0);
    //line.emplace_back(25, 25);
    //line.emplace_back(50, 50);
    //mapnik::geometry::point<double> centroid;
    //REQUIRE(mapnik::geometry::centroid(line, centroid));
    //REQUIRE(centroid.x == 25);
    //REQUIRE(centroid.y == 25);
}


SECTION("polygon") {

    //mapnik::geometry::polygon<double> poly;
    //mapnik::geometry::linear_ring<double> ring;
    //ring.emplace_back(0, 0);
    //ring.emplace_back(1, 0);
    //ring.emplace_back(1, 1);
    //ring.emplace_back(0, 1);
    //ring.emplace_back(0, 0);
    //poly.push_back(std::move(ring));

    //mapnik::geometry::point<double> centroid;
    //REQUIRE(mapnik::geometry::centroid(poly, centroid));
    ///REQUIRE(centroid.x == 0.5);
    //REQUIRE(centroid.y == 0.5);
}

}
