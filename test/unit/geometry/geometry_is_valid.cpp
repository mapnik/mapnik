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

SECTION("polygon ring tests") {

    mapnik::geometry::linear_ring<double> exterior;
    exterior.add_coord(0,0);
    exterior.add_coord(0,10);
    exterior.add_coord(-10,10);
    exterior.add_coord(-10,0);
    exterior.add_coord(0,0);

    mapnik::geometry::linear_ring<double> exterior_cw;
    exterior_cw.add_coord(0,0);
    exterior_cw.add_coord(-10,0);
    exterior_cw.add_coord(-10,10);
    exterior_cw.add_coord(0,10);
    exterior_cw.add_coord(0,0);

    mapnik::geometry::linear_ring<double> hole;
    hole.add_coord(-7,7);
    hole.add_coord(-3,7);
    hole.add_coord(-3,3);
    hole.add_coord(-7,3);
    hole.add_coord(-7,7);

    mapnik::geometry::linear_ring<double> outside_hole;
    outside_hole.add_coord(10,10);
    outside_hole.add_coord(10,5);
    outside_hole.add_coord(5,5);
    outside_hole.add_coord(5,10);
    outside_hole.add_coord(10,10);

    mapnik::geometry::linear_ring<double> overlap_hole;
    overlap_hole.add_coord(-3,3);
    overlap_hole.add_coord(-3,6);
    overlap_hole.add_coord(3,6);
    overlap_hole.add_coord(3,3);
    overlap_hole.add_coord(-3,3);

    mapnik::geometry::linear_ring<double> overlap_inner_1;
    overlap_inner_1.add_coord(-1,1);
    overlap_inner_1.add_coord(-6,1);
    overlap_inner_1.add_coord(-6,6);
    overlap_inner_1.add_coord(-1,6);
    overlap_inner_1.add_coord(-1,1);


    mapnik::geometry::linear_ring<double> overlap_inner_2;
    overlap_inner_2.add_coord(-9,9);
    overlap_inner_2.add_coord(-4,9);
    overlap_inner_2.add_coord(-4,4);
    overlap_inner_2.add_coord(-9,4);
    overlap_inner_2.add_coord(-9,9);


    std::cout << "Test 1" << std::endl;
    mapnik::geometry::polygon<double> correct;
    // add rings correctly and ensure we detect these as correct
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(hole);
        correct.set_exterior_ring(std::move(exterior2));
        correct.add_hole(std::move(hole2));
    }
    REQUIRE( mapnik::geometry::is_valid_rings(correct) );

    std::cout << "Test 2" << std::endl;
    mapnik::geometry::polygon<double> incorrect;
    // add rings incorrectly and ensure we detect these as incorrect
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(exterior2);
        incorrect.set_exterior_ring(std::move(hole2));
        incorrect.add_hole(std::move(exterior2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect) );

    std::cout << "Test 3" << std::endl;
    mapnik::geometry::polygon<double> incorrect2;
    // Rings added correctly, but interior is fully outside
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(outside_hole);
        incorrect2.set_exterior_ring(std::move(exterior2));
        incorrect2.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect2) );

    std::cout << "Test 4" << std::endl;
    mapnik::geometry::polygon<double> incorrect3;
    // Rings added correctly, but interior overlaps exterior
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(overlap_hole);
        incorrect3.set_exterior_ring(std::move(exterior2));
        incorrect3.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect3) );


    std::cout << "Test 5" << std::endl;
    mapnik::geometry::polygon<double> incorrect4;
    // Two valid interior rings overlap
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(overlap_inner_1);
        mapnik::geometry::linear_ring<double> hole3(overlap_inner_2);
        incorrect4.set_exterior_ring(std::move(exterior2));
        incorrect4.add_hole(std::move(hole2));
        incorrect4.add_hole(std::move(hole3));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect4) );


    std::cout << "Test 6" << std::endl;
    mapnik::geometry::polygon<double> correct2;;
    // Only has exterior ring, should be valid
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        correct2.set_exterior_ring(std::move(exterior2));
    }
    REQUIRE( mapnik::geometry::is_valid_rings(correct2) );


    std::cout << "Test 7" << std::endl;
    mapnik::geometry::polygon<double> correct3;;
    // Has no interior rings, has no exterior ring
    {
    }
    REQUIRE( mapnik::geometry::is_valid_rings(correct3) );


    std::cout << "Test 8" << std::endl;
    mapnik::geometry::polygon<double> correct4;
    // Has exterior ring, and 1 empty interior ring
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> empty_inner;
        correct4.set_exterior_ring(std::move(exterior2));
        correct4.add_hole(std::move(empty_inner));
    }
    REQUIRE( mapnik::geometry::is_valid_rings(correct4) );


    std::cout << "Test 9" << std::endl;
    mapnik::geometry::polygon<double> correct5;
    // Has no exterior ring, and 1 empty interior ring
    {
        mapnik::geometry::linear_ring<double> empty_inner;
        correct5.add_hole(std::move(empty_inner));
    }
    REQUIRE( mapnik::geometry::is_valid_rings(correct5) );


    std::cout << "Test 10" << std::endl;
    mapnik::geometry::polygon<double> incorrect5;
    // Has no exterior, has one non-empty inner
    {
        mapnik::geometry::linear_ring<double> hole2(overlap_hole);
        incorrect5.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect5) );


    mapnik::geometry::linear_ring<double> invalid_inner_winding;
    invalid_inner_winding.add_coord(-9,9);
    invalid_inner_winding.add_coord(-9,4);
    invalid_inner_winding.add_coord(-4,4);
    invalid_inner_winding.add_coord(-4,9);
    invalid_inner_winding.add_coord(-9,9);

    std::cout << "Test 11" << std::endl;
    mapnik::geometry::polygon<double> incorrect6;
    // Has an interior ring that's backwards
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(invalid_inner_winding);;
        incorrect6.set_exterior_ring(std::move(exterior2));
        incorrect6.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect6) );

    std::cout << "Test 12" << std::endl;
    mapnik::geometry::polygon<double> incorrect7;
    // Has an exterior ring that's backwards
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior_cw);
        mapnik::geometry::linear_ring<double> hole2(hole);;
        incorrect7.set_exterior_ring(std::move(exterior2));
        incorrect7.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect7) );


    mapnik::geometry::linear_ring<double> nonsimple_hole;
    nonsimple_hole.add_coord(-2,2);
    nonsimple_hole.add_coord(-4,2);
    nonsimple_hole.add_coord(-2,4);
    nonsimple_hole.add_coord(-4,4);
    nonsimple_hole.add_coord(-2,2);

    std::cout << "Test 13" << std::endl;
    mapnik::geometry::polygon<double> incorrect8;
    // Has an exterior ring that's backwards
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(nonsimple_hole);
        incorrect8.set_exterior_ring(std::move(exterior2));
        incorrect8.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect8) );

}

#endif // BOOST_VERSION >= 1.56

}
