#include "catch.hpp"
//#include "geometry_equal.hpp"

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/geometry_reprojection.hpp>
#include <mapnik/geometry_correct.hpp>

// std
#include <iostream>

TEST_CASE("geometry reprojection") {

SECTION("test_projection_4326_3857 - Empty Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans(source, dest);
    {
        geometry_empty geom;
        unsigned int err = 0;
        // Test Standard Transform
        geometry_empty new_geom = reproject_copy(geom, proj_trans, err);
        REQUIRE(err == 0);
        // Transform in reverse
        new_geom = reproject_copy(geom, proj_trans, err, true);
        REQUIRE(err == 0);
        // Transform providing projections not transfrom
        new_geom = reproject_copy(geom, source, dest, err);
        REQUIRE(err == 0);
        // Transform providing projections in reverse
        new_geom = reproject_copy(geom, dest, source, err);
        REQUIRE(err == 0);
        // Transform in place
        REQUIRE(reproject(new_geom, proj_trans));
        // Transform in place reverse
        REQUIRE(reproject(new_geom, proj_trans, true));
        // Transform in place providing projections
        REQUIRE(reproject(new_geom, source, dest));
        // Transform in place provoding projections reversed
        REQUIRE(reproject(new_geom, dest, source));
    }
} // End Section

SECTION("test_projection_4326_3857 - Empty Geometry in Geometry Variant") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans(source, dest);
    {
        geometry geom(std::move(geometry_empty()));
        unsigned int err = 0;
        // Test Standard Transform
        geometry new_geom = reproject_copy(geom, proj_trans, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
        // Transform in reverse
        new_geom = reproject_copy(geom, proj_trans, err, true);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
        // Transform providing projections not transfrom
        new_geom = reproject_copy(geom, source, dest, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
        // Transform providing projections in reverse
        new_geom = reproject_copy(geom, dest, source, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.is<geometry_empty>());
        // Transform in place
        REQUIRE(reproject(new_geom, proj_trans));
        // Transform in place reverse
        REQUIRE(reproject(new_geom, proj_trans, true));
        // Transform in place providing projections
        REQUIRE(reproject(new_geom, source, dest));
        // Transform in place provoding projections reversed
        REQUIRE(reproject(new_geom, dest, source));
    }
} // End Section

SECTION("test_projection_4326_3857 - Point Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans(source, dest);
    {
        point geom1(-97.552175, 35.522895);
        point geom2(-10859458.446776, 4235169.496066);
        unsigned int err = 0;
        // Test Standard Transform
        point new_geom = reproject_copy(geom1, proj_trans, err);
        std::cout << std::fixed << new_geom.x << "," << std::fixed << new_geom.y << std::endl;
        REQUIRE(err == 0);
        REQUIRE(new_geom.x == Approx(geom2.x));
        REQUIRE(new_geom.y == Approx(geom2.y));
        // Transform in reverse
        new_geom = reproject_copy(geom2, proj_trans, err, true);
        REQUIRE(err == 0);
        REQUIRE(new_geom.x == Approx(geom1.x));
        REQUIRE(new_geom.y == Approx(geom1.y));
        // Transform providing projections not transfrom
        new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.x == Approx(geom2.x));
        REQUIRE(new_geom.y == Approx(geom2.y));
        // Transform providing projections in reverse
        new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.x == Approx(geom1.x));
        REQUIRE(new_geom.y == Approx(geom1.y));
        // Transform in place
        point geom3(-97.552175, 35.522895);
        REQUIRE(reproject(geom3, proj_trans));
        REQUIRE(geom3.x == Approx(geom2.x));
        REQUIRE(geom3.y == Approx(geom2.y));
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans, true));
        REQUIRE(geom3.x == Approx(geom1.x));
        REQUIRE(geom3.y == Approx(geom1.y));
        // Transform in place providing projections
        REQUIRE(reproject(geom3, source, dest));
        REQUIRE(geom3.x == Approx(geom2.x));
        REQUIRE(geom3.y == Approx(geom2.y));
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        REQUIRE(geom3.x == Approx(geom1.x));
        REQUIRE(geom3.y == Approx(geom1.y));
    }
} // End Section
/*
SECTION("test_projection_4326_3857 - Point Geometry Object") {
    using namespace mapnik::geometry;
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest("+init=epsg:3857");
    mapnik::proj_transform proj_trans(source, dest);
    {
        geometry geom1(point(-97.552175, 35.522895));
        geometry geom2(point(-10859458.446776, 4235169.496066));
        unsigned int err = 0;
        // Test Standard Transform
        point new_geom = reproject_copy(geom1, proj_trans, err);
        std::cout << std::fixed << new_geom.x << "," << std::fixed << new_geom.y << std::endl;
        REQUIRE(err == 0);
        REQUIRE(new_geom.x == Approx(geom2_.x));
        REQUIRE(new_geom.y == Approx(geom2_.y));
        // Transform in reverse
        new_geom = reproject_copy(geom2, proj_trans, err, true);
        REQUIRE(err == 0);
        REQUIRE(new_geom.x == Approx(geom1.x));
        REQUIRE(new_geom.y == Approx(geom1.y));
        // Transform providing projections not transfrom
        new_geom = reproject_copy(geom1, source, dest, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.x == Approx(geom2.x));
        REQUIRE(new_geom.y == Approx(geom2.y));
        // Transform providing projections in reverse
        new_geom = reproject_copy(geom2, dest, source, err);
        REQUIRE(err == 0);
        REQUIRE(new_geom.x == Approx(geom1.x));
        REQUIRE(new_geom.y == Approx(geom1.y));
        // Transform in place
        point geom3(-97.552175, 35.522895);
        REQUIRE(reproject(geom3, proj_trans));
        REQUIRE(geom3.x == Approx(geom2.x));
        REQUIRE(geom3.y == Approx(geom2.y));
        // Transform in place reverse
        REQUIRE(reproject(geom3, proj_trans, true));
        REQUIRE(geom3.x == Approx(geom1.x));
        REQUIRE(geom3.y == Approx(geom1.y));
        // Transform in place providing projections
        REQUIRE(reproject(geom3, source, dest));
        REQUIRE(geom3.x == Approx(geom2.x));
        REQUIRE(geom3.y == Approx(geom2.y));
        // Transform in place provoding projections reversed
        REQUIRE(reproject(geom3, dest, source));
        REQUIRE(geom3.x == Approx(geom1.x));
        REQUIRE(geom3.y == Approx(geom1.y));
    }
} // End Section */
    /*{
        geometry geom(point(0,0));
        REQUIRE( mapnik::hit_test(geom,0,0,0) );
    }
    {
        geometry geom(point(0,0));
        REQUIRE( mapnik::hit_test(geom,1,0,1) );
    }
    {
        geometry geom(point(0,0));
        REQUIRE( mapnik::hit_test(geom,0,1,1) );
    }
    {
        geometry geom(point(0,0));
        REQUIRE( mapnik::hit_test(geom,1,1,1.5) );
    }
    {
        line_string line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        geometry geom(line);
        REQUIRE( mapnik::hit_test(geom,0,0,1.5) );
    }
    {
        line_string line;
        line.add_coord(0,0);
        line.add_coord(1,1);
        line.add_coord(2,2);
        multi_line_string multi_line;
        multi_line.emplace_back(std::move(line));
        geometry geom(multi_line);
        REQUIRE( mapnik::hit_test(geom,0,0,1.5) );
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
        REQUIRE( mapnik::hit_test(geom,-5,5,0) );

        multi_polygon mp;
        mp.push_back(poly);
        geometry geom_mp(mp);
        REQUIRE( mapnik::hit_test(geom_mp,-5,5,0) );

        correct(geom);
        REQUIRE( mapnik::hit_test(geom,-5,5,0) );
        correct(geom_mp);
        REQUIRE( mapnik::hit_test(geom_mp,-5,5,0) );

        geometry_collection gc;
        REQUIRE( !mapnik::hit_test(geometry(gc),-5,5,0) );
        gc.push_back(geom_mp);
        REQUIRE( mapnik::hit_test(geometry(gc),-5,5,0) );
        REQUIRE( !mapnik::hit_test(geometry(gc),-50,-50,0) );
        gc.emplace_back(point(-50,-50));
        REQUIRE( mapnik::hit_test(geometry(gc),-50,-50,0) );
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
        REQUIRE( !mapnik::hit_test(geom,-5,5,0) );
        // add another hole inside the first hole
        // which should be considered a hit
        linear_ring fill;
        fill.add_coord(-6,4);
        fill.add_coord(-6,6);
        fill.add_coord(-4,6);
        fill.add_coord(-4,4);
        fill.add_coord(-6,4);
        poly.add_hole(std::move(fill));
        REQUIRE( mapnik::hit_test(geometry(poly),-5,5,0) );
    }*/

} // End Testcase
