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


    std::string validation_message;


    mapnik::geometry::polygon<double> correct;
    // add rings correctly and ensure we detect these as correct
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(hole);
        correct.set_exterior_ring(std::move(exterior2));
        correct.add_hole(std::move(hole2));
    }
    REQUIRE( mapnik::geometry::is_valid_rings(correct, validation_message) );

    mapnik::geometry::polygon<double> incorrect;
    // add rings incorrectly and ensure we detect these as incorrect
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(exterior2);
        incorrect.set_exterior_ring(std::move(hole2));
        incorrect.add_hole(std::move(exterior2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect, validation_message) );

    mapnik::geometry::polygon<double> incorrect2;
    // Rings added correctly, but interior is fully outside
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(outside_hole);
        incorrect2.set_exterior_ring(std::move(exterior2));
        incorrect2.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect2, validation_message) );

    mapnik::geometry::polygon<double> incorrect3;
    // Rings added correctly, but interior overlaps exterior
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(overlap_hole);
        incorrect3.set_exterior_ring(std::move(exterior2));
        incorrect3.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect3, validation_message) );


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
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect4, validation_message) );


    mapnik::geometry::polygon<double> correct2;;
    // Only has exterior ring, should be valid
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        correct2.set_exterior_ring(std::move(exterior2));
    }
    REQUIRE( mapnik::geometry::is_valid_rings(correct2, validation_message) );


    mapnik::geometry::polygon<double> correct3;;
    // Has no interior rings, has no exterior ring
    {
    }
    REQUIRE( mapnik::geometry::is_valid_rings(correct3, validation_message) );


    mapnik::geometry::polygon<double> notcorrect4;
    // Has exterior ring, and 1 empty interior ring
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> empty_inner;
        notcorrect4.set_exterior_ring(std::move(exterior2));
        notcorrect4.add_hole(std::move(empty_inner));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(notcorrect4, validation_message) );


    mapnik::geometry::polygon<double> notcorrect5;
    // Has no exterior ring, and 1 empty interior ring
    {
        mapnik::geometry::linear_ring<double> empty_inner;
        notcorrect5.add_hole(std::move(empty_inner));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(notcorrect5, validation_message) );


    mapnik::geometry::polygon<double> incorrect5;
    // Has no exterior, has one non-empty inner
    {
        mapnik::geometry::linear_ring<double> hole2(overlap_hole);
        incorrect5.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect5, validation_message) );


    mapnik::geometry::linear_ring<double> invalid_inner_winding;
    invalid_inner_winding.add_coord(-9,9);
    invalid_inner_winding.add_coord(-9,4);
    invalid_inner_winding.add_coord(-4,4);
    invalid_inner_winding.add_coord(-4,9);
    invalid_inner_winding.add_coord(-9,9);

    mapnik::geometry::polygon<double> incorrect6;
    // Has an interior ring that's backwards
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(invalid_inner_winding);;
        incorrect6.set_exterior_ring(std::move(exterior2));
        incorrect6.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect6, validation_message) );

    mapnik::geometry::polygon<double> incorrect7;
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior_cw);
        mapnik::geometry::linear_ring<double> hole2(hole);;
        incorrect7.set_exterior_ring(std::move(exterior2));
        incorrect7.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect7, validation_message) );


    mapnik::geometry::linear_ring<double> nonsimple_hole;
    nonsimple_hole.add_coord(-2,2);
    nonsimple_hole.add_coord(-4,2);
    nonsimple_hole.add_coord(-2,4);
    nonsimple_hole.add_coord(-4,4);
    nonsimple_hole.add_coord(-2,2);

    mapnik::geometry::polygon<double> incorrect8;
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(nonsimple_hole);
        incorrect8.set_exterior_ring(std::move(exterior2));
        incorrect8.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect8, validation_message) );


    mapnik::geometry::linear_ring<double> exterior_repeated;
    exterior_repeated.add_coord(0,0);
    exterior_repeated.add_coord(0,10);
    exterior_repeated.add_coord(-10,10);
    exterior_repeated.add_coord(-10,0);
    exterior_repeated.add_coord(-10,0);
    exterior_repeated.add_coord(-10,0);
    exterior_repeated.add_coord(0,0);

    mapnik::geometry::linear_ring<double> hole_repeated;
    hole_repeated.add_coord(-7,7);
    hole_repeated.add_coord(-3,7);
    hole_repeated.add_coord(-3,3);
    hole_repeated.add_coord(-3,3);
    hole_repeated.add_coord(-3,3);
    hole_repeated.add_coord(-7,3);
    hole_repeated.add_coord(-7,7);

    mapnik::geometry::polygon<double> incorrect9;
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior_repeated);
        mapnik::geometry::linear_ring<double> hole2(hole);
        incorrect9.set_exterior_ring(std::move(exterior2));
        incorrect9.add_hole(std::move(hole2));
    }
    // TODO: these tests maybe shouldn't pass because of the repeated points....
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect9, validation_message) );

    mapnik::geometry::polygon<double> incorrect10;
    {
        mapnik::geometry::linear_ring<double> exterior2(exterior);
        mapnik::geometry::linear_ring<double> hole2(hole_repeated);
        incorrect10.set_exterior_ring(std::move(exterior2));
        incorrect10.add_hole(std::move(hole2));
    }
    // TODO: these tests maybe shouldn't pass because of the repeated points....
    //
    REQUIRE( !mapnik::geometry::is_valid_rings(incorrect10, validation_message) );




    // Test that reverse coordinates work when y-axis is flipped
    //
    mapnik::geometry::linear_ring<double> backwards_exterior;
    mapnik::geometry::linear_ring<double> backwards_inner;

    backwards_exterior.add_coord(0,0);
    backwards_exterior.add_coord(0,10);
    backwards_exterior.add_coord(10,10);
    backwards_exterior.add_coord(10,0);
    backwards_exterior.add_coord(0,0);

    backwards_inner.add_coord(8,8);
    backwards_inner.add_coord(5,8);
    backwards_inner.add_coord(5,5);
    backwards_inner.add_coord(8,5);
    backwards_inner.add_coord(8,8);

    mapnik::geometry::polygon<double> correct6;
    {
        mapnik::geometry::linear_ring<double> exterior2(backwards_exterior);
        mapnik::geometry::linear_ring<double> hole2(backwards_inner);
        correct6.set_exterior_ring(std::move(exterior2));
        correct6.add_hole(std::move(hole2));
    }
    REQUIRE( !mapnik::geometry::is_valid_rings(correct6, validation_message) );
    REQUIRE( mapnik::geometry::is_valid_rings(correct6, validation_message, mapnik::geometry::Y_AXIS_NORTH_NEGATIVE) );



}

#endif // BOOST_VERSION >= 1.56

}
