#include "catch.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/geometry/is_valid.hpp>

TEST_CASE("geometry is_valid") {

// only Boost >= 1.56 has the is_valid function, but only after 1.58 is there support for returning what is invalid
#if BOOST_VERSION >= 105800


SECTION("empty geometry") {
    mapnik::geometry::geometry_empty empty;
    CHECK( mapnik::geometry::is_valid(empty) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(empty, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(empty, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("point") {
    mapnik::geometry::point<double> pt(0,0);
    CHECK( mapnik::geometry::is_valid(pt) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(pt, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(pt, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("point -- geometry object") {
    mapnik::geometry::point<double> pt(0,0);
    mapnik::geometry::geometry<double> geom(pt);
    CHECK( mapnik::geometry::is_valid(geom) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(geom, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(geom, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

#if BOOST_VERSION < 106000

SECTION("point unitialized") {
    mapnik::geometry::point<double> pt2;
    CHECK( mapnik::geometry::is_valid(pt2) );
    std::string message2;
    CHECK( mapnik::geometry::is_valid(pt2, message2) );
    CHECK( message2 == "Geometry is valid");
    boost::geometry::validity_failure_type failure2;
    CHECK( mapnik::geometry::is_valid(pt2, failure2) );
    CHECK( failure2 == boost::geometry::no_failure );
}
#endif

#if BOOST_VERSION >= 106000

SECTION("point NaN") {
    mapnik::geometry::point<double> pt(std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::quiet_NaN());
    CHECK( std::isnan(pt.x) );
    CHECK( std::isnan(pt.y) );
    CHECK( !mapnik::geometry::is_valid(pt) );
    std::string message;
    CHECK( !mapnik::geometry::is_valid(pt, message) );
    CHECK( message == "Geometry has point(s) with invalid coordinate(s)");
    boost::geometry::validity_failure_type failure;
    CHECK( !mapnik::geometry::is_valid(pt, failure) );
    CHECK( failure == boost::geometry::failure_invalid_coordinate );
}

SECTION("point Infinity") {
    mapnik::geometry::point<double> pt(std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());
    CHECK( std::isinf(pt.x) );
    CHECK( std::isinf(pt.y) );
    CHECK( !mapnik::geometry::is_valid(pt) );
    std::string message;
    CHECK( !mapnik::geometry::is_valid(pt, message) );
    CHECK( message == "Geometry has point(s) with invalid coordinate(s)");
    boost::geometry::validity_failure_type failure;
    CHECK( !mapnik::geometry::is_valid(pt, failure) );
    CHECK( failure == boost::geometry::failure_invalid_coordinate );
}

#else // BOOST_VERSION >= 1.60

// This is funky that boost geometry is_valid does not check for NAN when dealing with a point
// this test is here in case the logic ever changes
// Bug report on this: https://svn.boost.org/trac/boost/ticket/11711
SECTION("point NaN") {
    mapnik::geometry::point<double> pt(std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::quiet_NaN());
    CHECK( std::isnan(pt.x) );
    CHECK( std::isnan(pt.y) );
    CHECK( mapnik::geometry::is_valid(pt) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(pt, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(pt, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

// This is funky that boost geometry is_valid does not check for infinity when dealing with a point
// this test is here in case the logic ever changes
// Bug report on this: https://svn.boost.org/trac/boost/ticket/11711
SECTION("point Infinity") {
    mapnik::geometry::point<double> pt(std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());
    CHECK( std::isinf(pt.x) );
    CHECK( std::isinf(pt.y) );
    CHECK( mapnik::geometry::is_valid(pt) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(pt, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(pt, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

#endif // BOOST_VERSION >= 1.60

SECTION("multi point") {
    mapnik::geometry::multi_point<double> mpt;
    mpt.emplace_back(0,0);
    mpt.emplace_back(1,1);
    CHECK( mapnik::geometry::is_valid(mpt) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(mpt, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(mpt, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("multi point empty") {
    mapnik::geometry::multi_point<double> mpt;
    CHECK( mapnik::geometry::is_valid(mpt) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(mpt, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(mpt, failure) );
    CHECK( failure == boost::geometry::no_failure );
}


SECTION("line_string") {
    mapnik::geometry::line_string<double> line;
    line.emplace_back(0,0);
    line.emplace_back(1,1);
    CHECK( mapnik::geometry::is_valid(line) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(line, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(line, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

// This shouldn't fail -- test added in case logic ever changes
SECTION("line_string repeated points") {
    mapnik::geometry::line_string<double> line;
    line.emplace_back(0,0);
    line.emplace_back(1,1);
    line.emplace_back(1,1);
    line.emplace_back(2,2);
    CHECK( mapnik::geometry::is_valid(line) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(line, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(line, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("line_string empty") {
    mapnik::geometry::line_string<double> line;
    CHECK( !mapnik::geometry::is_valid(line) );
    std::string message;
    CHECK( !mapnik::geometry::is_valid(line, message) );
    CHECK( message == "Geometry has too few points");
    boost::geometry::validity_failure_type failure;
    CHECK( !mapnik::geometry::is_valid(line, failure) );
    CHECK( failure == boost::geometry::failure_few_points );
}

SECTION("multi_line_string") {
    mapnik::geometry::line_string<double> line1;
    line1.emplace_back(0,0);
    line1.emplace_back(1,1);
    mapnik::geometry::line_string<double> line2;
    line2.emplace_back(0,1);
    line2.emplace_back(1,2);
    mapnik::geometry::multi_line_string<double> lines;
    lines.emplace_back(line1);
    lines.emplace_back(line2);
    CHECK( mapnik::geometry::is_valid(lines) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(lines, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(lines, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("multi_line_string empty") {
    mapnik::geometry::multi_line_string<double> lines;
    CHECK( mapnik::geometry::is_valid(lines) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(lines, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(lines, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("polygon") {
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(1,0);
    ring.emplace_back(1,1);
    ring.emplace_back(0,1);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    CHECK( mapnik::geometry::is_valid(poly) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(poly, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(poly, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("polygon invalid winding order") {
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(0,1);
    ring.emplace_back(1,1);
    ring.emplace_back(1,0);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    CHECK( !mapnik::geometry::is_valid(poly) );
    std::string message;
    CHECK( !mapnik::geometry::is_valid(poly, message) );
    CHECK( message == "Geometry has wrong orientation" );
    boost::geometry::validity_failure_type failure;
    CHECK( !mapnik::geometry::is_valid(poly, failure) );
    CHECK( failure == boost::geometry::failure_wrong_orientation );
}

// repeated points are not considered invalid in a polygon
SECTION("polygon 2 repeated points") {
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(1,0);
    ring.emplace_back(1,1);
    ring.emplace_back(1,1);
    ring.emplace_back(0,1);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    CHECK( mapnik::geometry::is_valid(poly) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(poly, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(poly, failure) );
    CHECK( failure == boost::geometry::no_failure );
}
// repeated points are not considered invalid in a polygon
SECTION("polygon 3 repeated points") {
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(1,0);
    ring.emplace_back(1,1);
    ring.emplace_back(1,1);
    ring.emplace_back(1,1);
    ring.emplace_back(0,1);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    CHECK( mapnik::geometry::is_valid(poly) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(poly, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(poly, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("polygon that is empty") {
    mapnik::geometry::polygon<double> poly;
    poly.emplace_back();
    CHECK( !mapnik::geometry::is_valid(poly) );
    std::string message;
    CHECK( !mapnik::geometry::is_valid(poly, message) );
    CHECK( message == "Geometry has too few points");
    boost::geometry::validity_failure_type failure;
    CHECK( !mapnik::geometry::is_valid(poly, failure) );
    CHECK( failure == boost::geometry::failure_few_points );
}

SECTION("polygon with spike") {
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(1,0);
    ring.emplace_back(1,1);
    ring.emplace_back(2,2);
    ring.emplace_back(1,1);
    ring.emplace_back(0,1);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    CHECK( !mapnik::geometry::is_valid(poly) );
    std::string message;
    CHECK( !mapnik::geometry::is_valid(poly, message) );
    CHECK( message == "Geometry has spikes. A spike point was found with apex at (2, 2)");
    boost::geometry::validity_failure_type failure;
    CHECK( !mapnik::geometry::is_valid(poly, failure) );
    CHECK( failure == boost::geometry::failure_spikes );
}

SECTION("polygon with hole") {
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(3,0);
    ring.emplace_back(3,3);
    ring.emplace_back(0,3);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    mapnik::geometry::linear_ring<double> hole;
    hole.emplace_back(1,1);
    hole.emplace_back(1,2);
    hole.emplace_back(2,2);
    hole.emplace_back(2,1);
    hole.emplace_back(1,1);
    poly.push_back(std::move(hole));
    CHECK( mapnik::geometry::is_valid(poly) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(poly, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(poly, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("polygon with empty hole") {
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(3,0);
    ring.emplace_back(3,3);
    ring.emplace_back(0,3);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    mapnik::geometry::linear_ring<double> hole;
    poly.push_back(std::move(hole));
    CHECK( !mapnik::geometry::is_valid(poly) );
    std::string message;
    CHECK( !mapnik::geometry::is_valid(poly, message) );
    CHECK( message == "Geometry has too few points");
    boost::geometry::validity_failure_type failure;
    CHECK( !mapnik::geometry::is_valid(poly, failure) );
    CHECK( failure == boost::geometry::failure_few_points );
}


SECTION("polygon with hole with invalid winding order") {
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(3,0);
    ring.emplace_back(3,3);
    ring.emplace_back(0,3);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    mapnik::geometry::linear_ring<double> hole;
    hole.emplace_back(1,1);
    hole.emplace_back(2,1);
    hole.emplace_back(2,2);
    hole.emplace_back(1,2);
    hole.emplace_back(1,1);
    poly.push_back(std::move(hole));
    CHECK( !mapnik::geometry::is_valid(poly) );
    std::string message;
    CHECK( !mapnik::geometry::is_valid(poly, message) );
    CHECK( message == "Geometry has wrong orientation" );
    boost::geometry::validity_failure_type failure;
    CHECK( !mapnik::geometry::is_valid(poly, failure) );
    CHECK( failure == boost::geometry::failure_wrong_orientation );
}

SECTION("multi polygon") {
    mapnik::geometry::multi_polygon<double> mp;
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(1,0);
    ring.emplace_back(1,1);
    ring.emplace_back(0,1);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    mapnik::geometry::polygon<double> poly2;
    mapnik::geometry::linear_ring<double> ring2;
    ring2.emplace_back(0,0);
    ring2.emplace_back(-1,0);
    ring2.emplace_back(-1,-1);
    ring2.emplace_back(0,-1);
    ring2.emplace_back(0,0);
    poly2.push_back(std::move(ring2));
    mp.emplace_back(poly);
    mp.emplace_back(poly2);
    CHECK( mapnik::geometry::is_valid(mp) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(mp, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(mp, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("multi polygon with hole") {
    mapnik::geometry::multi_polygon<double> mp;
    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0,0);
    ring.emplace_back(3,0);
    ring.emplace_back(3,3);
    ring.emplace_back(0,3);
    ring.emplace_back(0,0);
    poly.push_back(std::move(ring));
    mapnik::geometry::linear_ring<double> hole;
    hole.emplace_back(1,1);
    hole.emplace_back(1,2);
    hole.emplace_back(2,2);
    hole.emplace_back(2,1);
    hole.emplace_back(1,1);
    poly.push_back(std::move(hole));
    mapnik::geometry::polygon<double> poly2;
    mapnik::geometry::linear_ring<double> ring2;
    ring2.emplace_back(0,0);
    ring2.emplace_back(-3,0);
    ring2.emplace_back(-3,-3);
    ring2.emplace_back(0,-3);
    ring2.emplace_back(0,0);
    poly2.push_back(std::move(ring2));
    mapnik::geometry::linear_ring<double> hole2;
    hole2.emplace_back(-1,-1);
    hole2.emplace_back(-1,-2);
    hole2.emplace_back(-2,-2);
    hole2.emplace_back(-2,-1);
    hole2.emplace_back(-1,-1);
    poly2.push_back(std::move(hole2));
    mp.emplace_back(poly);
    mp.emplace_back(poly2);
    CHECK( mapnik::geometry::is_valid(mp) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(mp, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(mp, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

SECTION("multi polygon empty") {
    mapnik::geometry::multi_polygon<double> mp;
    CHECK( mapnik::geometry::is_valid(mp) );
    std::string message;
    CHECK( mapnik::geometry::is_valid(mp, message) );
    CHECK( message == "Geometry is valid");
    boost::geometry::validity_failure_type failure;
    CHECK( mapnik::geometry::is_valid(mp, failure) );
    CHECK( failure == boost::geometry::no_failure );
}

#endif // BOOST_VERSION >= 1.58

}
