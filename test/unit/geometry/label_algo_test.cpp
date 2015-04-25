#include "catch.hpp"

#include <iostream>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <mapnik/geometry_centroid.hpp>
#include <vector>
#include <algorithm>

TEST_CASE("labeling") {

SECTION("algorithms") {

    try
    {
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
        // TODO - centroid and interior should be equal but they appear not to be (check largest)
        // MULTIPOLYGON(((-52 40,-60 32,-68 40,-60 48,-52 40)),((-60 50,-80 30,-100 49.9999999999999,-80.0000000000001 70,-60 50)),((-52 60,-60 52,-68 60,-60 68,-52 60)))
#if 0
        // hit tests
        {
            mapnik::geometry_type pt_hit(mapnik::geometry::geometry_types::Point);
            pt_hit.move_to(10,10);
            mapnik::vertex_adapter va(pt_hit);
            REQUIRE( mapnik::label::hit_test(va, 10, 10, 0.1) );
            REQUIRE( !mapnik::label::hit_test(va, 9, 9, 0) );
            REQUIRE( mapnik::label::hit_test(va, 9, 9, 1.5) );
        }
        {
            mapnik::geometry_type line_hit(mapnik::geometry::geometry_types::LineString);
            line_hit.move_to(0,0);
            line_hit.line_to(50,50);
            mapnik::vertex_adapter va(line_hit);
            REQUIRE( mapnik::label::hit_test(va, 0, 0, 0.001) );
            REQUIRE( !mapnik::label::hit_test(va, 1, 1, 0) );
            REQUIRE( mapnik::label::hit_test(va, 1, 1, 1.001) );
        }
#endif
    }
    catch (std::exception const & ex)
    {
        std::clog << ex.what() << "\n";
        REQUIRE(false);
    }
}
}
