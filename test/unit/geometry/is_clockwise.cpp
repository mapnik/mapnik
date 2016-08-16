#include "catch.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/util/is_clockwise.hpp>

TEST_CASE("Ring is_clockwise") {

    // Input is rather thin triangle to test precision issues aren't getting in the way.
    SECTION("Clockwise")
    {
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(-13499697.0366658326, 4698431.85179749783);
        ring.emplace_back(-13499697.1113113686, 4698431.85179749783);
        ring.emplace_back(-13499697.0366658326, 4698431.92644303292);
        ring.emplace_back(-13499697.0366658326, 4698431.85179749783);
        REQUIRE(mapnik::util::is_clockwise(ring) == true);
    }
    SECTION("Anti-Clockwise")
    {
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(-13499697.0366658326, 4698431.85179749783);
        ring.emplace_back(-13499697.0366658326, 4698431.92644303292);
        ring.emplace_back(-13499697.1113113686, 4698431.85179749783);
        ring.emplace_back(-13499697.0366658326, 4698431.85179749783);
        REQUIRE(mapnik::util::is_clockwise(ring) == false);
    }
}
