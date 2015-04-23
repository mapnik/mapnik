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

#endif // BOOST_VERSION >= 1.56

}
