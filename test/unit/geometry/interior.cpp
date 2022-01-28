#include "catch.hpp"

#include <mapnik/geometry/interior.hpp>

TEST_CASE("polygon interior")
{
    SECTION("empty polygon")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::point<double> pt;

        CHECK(!mapnik::geometry::interior(poly, 1.0, pt));
    }

    SECTION("empty exterior ring")
    {
        mapnik::geometry::polygon<double> poly;
        poly.emplace_back();

        mapnik::geometry::point<double> pt;

        CHECK(!mapnik::geometry::interior(poly, 1.0, pt));
    }

    SECTION("interior of a square")
    {
        mapnik::geometry::polygon<double> poly;
        poly.emplace_back();
        auto& exterior_ring = poly.front();
        exterior_ring.emplace_back(-1, -1);
        exterior_ring.emplace_back(1, -1);
        exterior_ring.emplace_back(1, 1);
        exterior_ring.emplace_back(-1, 1);
        exterior_ring.emplace_back(-1, -1);

        mapnik::geometry::point<double> pt{-3, -3};

        CHECK(mapnik::geometry::interior(poly, 1.0, pt));
        CHECK(pt.x == Approx(0));
        CHECK(pt.y == Approx(0));
    }
}
