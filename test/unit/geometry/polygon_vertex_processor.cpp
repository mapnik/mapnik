#include "catch.hpp"
#include "unit/vertex_adapter/fake_path.hpp"

#include <mapnik/geometry/polygon_vertex_processor.hpp>

TEST_CASE("polygon_vertex_processor") {

SECTION("empty polygon") {

    fake_path path = {};
    mapnik::geometry::polygon_vertex_processor<double> proc;
    proc.add_path(path);
    CHECK(proc.polygon_.size() == 0);
}

SECTION("empty outer ring") {

    fake_path path = {};
    path.vertices_.emplace_back(0, 0, mapnik::SEG_CLOSE);
    path.rewind(0);

    mapnik::geometry::polygon_vertex_processor<double> proc;
    proc.add_path(path);
    REQUIRE(proc.polygon_.size() == 1);
    REQUIRE(proc.polygon_.front().size() == 0);
}

SECTION("empty inner ring") {

    fake_path path = {};
    path.vertices_.emplace_back(-1, -1, mapnik::SEG_MOVETO);
    path.vertices_.emplace_back( 1, -1, mapnik::SEG_LINETO);
    path.vertices_.emplace_back( 1,  1, mapnik::SEG_LINETO);
    path.vertices_.emplace_back(-1,  1, mapnik::SEG_LINETO);
    path.vertices_.emplace_back( 0,  0, mapnik::SEG_CLOSE);
    path.vertices_.emplace_back( 0,  0, mapnik::SEG_CLOSE);
    path.rewind(0);

    mapnik::geometry::polygon_vertex_processor<double> proc;
    proc.add_path(path);

    REQUIRE(proc.polygon_.size() == 2);
    auto const& outer_ring = proc.polygon_.front();
    REQUIRE(outer_ring.size() == 5);

    CHECK(outer_ring[0].x == Approx(-1));
    CHECK(outer_ring[0].y == Approx(-1));

    CHECK(outer_ring[1].x == Approx( 1));
    CHECK(outer_ring[1].y == Approx(-1));

    CHECK(outer_ring[2].x == Approx( 1));
    CHECK(outer_ring[2].y == Approx( 1));

    CHECK(outer_ring[3].x == Approx(-1));
    CHECK(outer_ring[3].y == Approx( 1));

    CHECK(outer_ring[4].x == Approx(-1));
    CHECK(outer_ring[4].y == Approx(-1));
}

}
