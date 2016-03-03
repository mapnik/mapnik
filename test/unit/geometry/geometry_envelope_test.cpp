
#include "catch.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_envelope.hpp>
#include <mapnik/geometry_correct.hpp>

TEST_CASE("geometry ops - envelope") {

SECTION("envelope_test - double") {
    using namespace mapnik::geometry;
    {
        geometry<double> geom(point<double>(1,2));
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE( bbox.minx() == 1 );
        REQUIRE( bbox.miny() == 2 );
        REQUIRE( bbox.maxx() == 1 );
        REQUIRE( bbox.maxy() == 2 );
    }
    {
        // Test empty geom
        geometry<double> geom = mapnik::geometry::geometry_empty();
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE_FALSE( bbox.valid() );
    }
    {
        line_string<double> line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        geometry<double> geom(line);
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE( bbox.minx() == 0 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 2 );
        REQUIRE( bbox.maxy() == 2 );
    }
    {
        line_string<double> line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        line_string<double> line2;
        line2.add_coord(0,0);
        line2.add_coord(-1,-1);
        line2.add_coord(-2,-2);
        multi_line_string<double> multi_line;
        multi_line.emplace_back(std::move(line));
        multi_line.emplace_back(std::move(line2));
        geometry<double> geom(multi_line);
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE( bbox.minx() == -2 );
        REQUIRE( bbox.miny() == -2 );
        REQUIRE( bbox.maxx() == 2 );
        REQUIRE( bbox.maxy() == 2 );
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
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );

        multi_polygon<double> mp;
        mp.push_back(poly);
        geometry<double> geom_mp(mp);
        bbox = mapnik::geometry::envelope(geom_mp);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );

        correct(geom);
        bbox = mapnik::geometry::envelope(geom);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );
        correct(geom_mp);
        bbox = mapnik::geometry::envelope(geom_mp);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );

        geometry_collection<double> gc;
        bbox = mapnik::geometry::envelope(gc);
        REQUIRE_FALSE( bbox.valid() );
        gc.push_back(geom_mp);
        bbox = mapnik::geometry::envelope(gc);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );
        gc.emplace_back(point<double>(-50,-50));
        bbox = mapnik::geometry::envelope(gc);
        REQUIRE( bbox.minx() == -50 );
        REQUIRE( bbox.miny() == -50 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );
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
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(poly);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );
        // add another hole inside the first hole
        // which should be considered a hit
        linear_ring<double> fill;
        fill.add_coord(-6,4);
        fill.add_coord(-6,6);
        fill.add_coord(-4,6);
        fill.add_coord(-4,4);
        fill.add_coord(-6,4);
        poly.add_hole(std::move(fill));
        bbox = mapnik::geometry::envelope(poly);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );
    }
}

}
