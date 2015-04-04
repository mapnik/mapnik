#include "catch.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_envelope.hpp>
#include <mapnik/geometry_correct.hpp>

TEST_CASE("geometry ops - envelope") {

SECTION("envelope_test") {
    using namespace mapnik::geometry;
    {
        geometry geom(point(1,2));
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE( bbox.minx() == 1 );
        REQUIRE( bbox.miny() == 2 );
        REQUIRE( bbox.maxx() == 1 );
        REQUIRE( bbox.maxy() == 2 );
    }
    {
        // Test empty geom
        geometry geom(std::move(mapnik::geometry::geometry_empty()));
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE_FALSE( bbox.valid() );
    }
    {
        line_string line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        geometry geom(line);
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE( bbox.minx() == 0 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 2 );
        REQUIRE( bbox.maxy() == 2 );
    }
    {
        line_string line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        line_string line2;
        line2.add_coord(0,0);
        line2.add_coord(-1,-1);
        line2.add_coord(-2,-2);
        multi_line_string multi_line;
        multi_line.emplace_back(std::move(line));
        multi_line.emplace_back(std::move(line2));
        geometry geom(multi_line);
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE( bbox.minx() == -2 );
        REQUIRE( bbox.miny() == -2 );
        REQUIRE( bbox.maxx() == 2 );
        REQUIRE( bbox.maxy() == 2 );
    }
    {
        polygon poly;
        linear_ring ring;
        ring.add_coord(0,0);
        ring.add_coord(-10,0);
        ring.add_coord(-10,10);
        ring.add_coord(0,10);
        ring.add_coord(0,0);
        poly.set_exterior_ring(std::move(ring));
        geometry geom(poly);
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(geom);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );

        multi_polygon mp;
        mp.push_back(poly);
        geometry geom_mp(mp);
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

        geometry_collection gc;
        bbox = mapnik::geometry::envelope(gc);
        REQUIRE_FALSE( bbox.valid() );
        gc.push_back(geom_mp);
        bbox = mapnik::geometry::envelope(gc);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );
        gc.emplace_back(point(-50,-50));
        bbox = mapnik::geometry::envelope(gc);
        REQUIRE( bbox.minx() == -50 );
        REQUIRE( bbox.miny() == -50 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );
    }

    {
        // polygon with hole
        polygon poly;
        linear_ring ring;
        ring.add_coord(0,0);
        ring.add_coord(-10,0);
        ring.add_coord(-10,10);
        ring.add_coord(0,10);
        ring.add_coord(0,0);
        poly.set_exterior_ring(std::move(ring));
        linear_ring hole;
        hole.add_coord(-7,7);
        hole.add_coord(-7,3);
        hole.add_coord(-3,3);
        hole.add_coord(-3,7);
        hole.add_coord(-7,7);
        poly.add_hole(std::move(hole));
        geometry geom(poly);
        mapnik::box2d<double> bbox = mapnik::geometry::envelope(poly);
        REQUIRE( bbox.minx() == -10 );
        REQUIRE( bbox.miny() == 0 );
        REQUIRE( bbox.maxx() == 0 );
        REQUIRE( bbox.maxy() == 10 );
        // add another hole inside the first hole
        // which should be considered a hit
        linear_ring fill;
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
