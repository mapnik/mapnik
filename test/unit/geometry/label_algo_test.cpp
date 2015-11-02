#include "catch.hpp"

#include <iostream>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_centroid.hpp>
#include <algorithm>

TEST_CASE("labeling") {

SECTION("algorithms") {

    // reused these for simplicity
    mapnik::geometry::point<double> centroid;
    {
        // single point
        mapnik::geometry::point<double> pt(10,10);
        REQUIRE( mapnik::geometry::centroid(pt, centroid));
        REQUIRE( pt.x == centroid.x);
        REQUIRE( pt.y == centroid.y);
    }

    // linestring with three consecutive verticies
    {
        mapnik::geometry::line_string<double> line;
        line.add_coord(0, 0);
        line.add_coord(25, 25);
        line.add_coord(50, 50);
        REQUIRE(mapnik::geometry::centroid(line, centroid));
        REQUIRE( centroid.x == 25 );
        REQUIRE( centroid.y == 25 );
    }
}
}
