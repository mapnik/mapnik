#include "catch.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <mapnik/geometry_is_valid.hpp>

TEST_CASE("geometry is_valid") {

// only Boost >= 1.56 has the is_valid function
#if BOOST_VERSION >= 105600

SECTION("point") {
    mapnik::geometry::point<double> pt(0,0);
    REQUIRE( mapnik::geometry::is_valid(pt) );

    // uninitialized: should likely not be considered valid
    mapnik::geometry::point<double> pt2;
    REQUIRE( mapnik::geometry::is_valid(pt2) );
}

SECTION("line_string") {
    mapnik::geometry::line_string<double> line;
    line.add_coord(0,0);    
    line.add_coord(1,1);
    REQUIRE( mapnik::geometry::is_valid(line) );

}

SECTION("polygon with incorrect storage of exterior and interior rings") {

    mapnik::geometry::linear_ring<double> exterior;
    exterior.add_coord(0,0);
    exterior.add_coord(-10,0);
    exterior.add_coord(-10,10);
    exterior.add_coord(0,10);
    exterior.add_coord(0,0);
    mapnik::geometry::linear_ring<double> hole;
    hole.add_coord(-7,7);
    hole.add_coord(-7,3);
    hole.add_coord(-3,3);
    hole.add_coord(-3,7);
    hole.add_coord(-7,7);

    mapnik::geometry::linear_ring<double> outside_hole;
    outside_hole.add_coord(10,10);
    outside_hole.add_coord(10,5);
    outside_hole.add_coord(5,5);
    outside_hole.add_coord(5,10);
    outside_hole.add_coord(10,10);

    mapnik::geometry::linear_ring<double> overlap_hole;
    overlap_hole.add_coord(-3,3);
    overlap_hole.add_coord(3,3);
    overlap_hole.add_coord(3,6);
    overlap_hole.add_coord(-3,6);
    overlap_hole.add_coord(-3,3);

    mapnik::geometry::polygon<double> correct;
    // add rings correctly and ensure we detect these as correct
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(hole);
        correct.set_exterior_ring(std::move(exterior2));
        correct.add_hole(std::move(hole2));
    }
    REQUIRE( mapnik::geometry::is_valid_rings(correct) );

    mapnik::geometry::polygon<double> incorrect;
    // add rings incorrectly and ensure we detect these as incorrect
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(exterior2);
        incorrect.set_exterior_ring(std::move(hole2));
        incorrect.add_hole(std::move(exterior2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect) );

    mapnik::geometry::polygon<double> incorrect2;
    // Rings added correctly, but interior is fully outside
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(outside_hole);
        incorrect2.set_exterior_ring(std::move(exterior2));
        incorrect2.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect2) );

    mapnik::geometry::polygon<double> incorrect3;
    // Rings added correctly, but interior overlaps exterior
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(overlap_hole);
        incorrect3.set_exterior_ring(std::move(exterior2));
        incorrect3.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect3) );


}

#endif // BOOST_VERSION >= 1.56

}
