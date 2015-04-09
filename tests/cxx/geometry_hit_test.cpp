#include "catch.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/hit_test_filter.hpp>
#include <mapnik/geometry_correct.hpp>

TEST_CASE("geometry ops") {

SECTION("hit_test_filter - double") {
    using namespace mapnik::geometry;
    {
        geometry<double> geom(point<double>(0,0));
        REQUIRE( mapnik::hit_test(geom,0,0,0) );
    }
    {
        geometry<double> geom(point<double>(0,0));
        REQUIRE( mapnik::hit_test(geom,1,0,1) );
    }
    {
        geometry<double> geom(point<double>(0,0));
        REQUIRE( mapnik::hit_test(geom,0,1,1) );
    }
    {
        geometry<double> geom(point<double>(0,0));
        REQUIRE( mapnik::hit_test(geom,1,1,1.5) );
    }
    {
        line_string<double> line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        geometry<double> geom(line);
        REQUIRE( mapnik::hit_test(geom,0,0,1.5) );
    }
    {
        line_string<double> line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        multi_line_string<double> multi_line;
        multi_line.emplace_back(std::move(line));
        geometry<double> geom(multi_line);
        REQUIRE( mapnik::hit_test(geom,0,0,1.5) );
    }
    {
        polygon<double> poly;
        linear_ring<double> ring;
        ring.add_coord(0,0);
        ring.add_coord(-10,0);
        ring.add_coord(-10,10);
        ring.add_coord(0,10);
        ring.add_coord(0,0);
        poly.set_exterior_ring(std::move(ring));
        geometry<double> geom(poly);
        REQUIRE( mapnik::hit_test(geom,-5,5,0) );

        multi_polygon<double> mp;
        mp.push_back(poly);
        geometry<double> geom_mp(mp);
        REQUIRE( mapnik::hit_test(geom_mp,-5,5,0) );

        correct(geom);
        REQUIRE( mapnik::hit_test(geom,-5,5,0) );
        correct(geom_mp);
        REQUIRE( mapnik::hit_test(geom_mp,-5,5,0) );

        geometry_collection<double> gc;
        REQUIRE( !mapnik::hit_test(geometry<double>(gc),-5,5,0) );
        gc.push_back(geom_mp);
        REQUIRE( mapnik::hit_test(geometry<double>(gc),-5,5,0) );
        REQUIRE( !mapnik::hit_test(geometry<double>(gc),-50,-50,0) );
        gc.emplace_back(point<double>(-50,-50));
        REQUIRE( mapnik::hit_test(geometry<double>(gc),-50,-50,0) );
    }

    {
        // polygon with hole
        polygon<double> poly;
        linear_ring<double> ring;
        ring.add_coord(0,0);
        ring.add_coord(-10,0);
        ring.add_coord(-10,10);
        ring.add_coord(0,10);
        ring.add_coord(0,0);
        poly.set_exterior_ring(std::move(ring));
        linear_ring<double> hole;
        hole.add_coord(-7,7);
        hole.add_coord(-7,3);
        hole.add_coord(-3,3);
        hole.add_coord(-3,7);
        hole.add_coord(-7,7);
        poly.add_hole(std::move(hole));
        geometry<double> geom(poly);
        REQUIRE( !mapnik::hit_test(geom,-5,5,0) );
        // add another hole inside the first hole
        // which should be considered a hit
        linear_ring<double> fill;
        fill.add_coord(-6,4);
        fill.add_coord(-6,6);
        fill.add_coord(-4,6);
        fill.add_coord(-4,4);
        fill.add_coord(-6,4);
        poly.add_hole(std::move(fill));
        REQUIRE( mapnik::hit_test(geometry<double>(poly),-5,5,0) );
    }
}

}
