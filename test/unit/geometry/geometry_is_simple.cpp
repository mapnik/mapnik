#include "catch.hpp"

#include <boost/version.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/is_simple.hpp>

TEST_CASE("geometry is_simple")
{
// only Boost >= 1.58 has the required is_valid function version
#if BOOST_VERSION >= 105800

    SECTION("point")
    {
        mapnik::geometry::geometry_empty empty;
        CHECK(mapnik::geometry::is_simple(empty));
    }

    SECTION("point")
    {
        mapnik::geometry::point<double> pt(0, 0);
        CHECK(mapnik::geometry::is_simple(pt));
    }

    SECTION("point uninitialized")
    {
        mapnik::geometry::point<double> pt2;
        CHECK(mapnik::geometry::is_simple(pt2));
    }

    SECTION("point -- geometry object")
    {
        mapnik::geometry::point<double> pt(0, 0);
        mapnik::geometry::geometry<double> geom(pt);
        CHECK(mapnik::geometry::is_simple(geom));
    }

    // This is funky that boost geometry is_simple does not check for NAN when dealing with a point
    // this test is here in case the logic ever changes
    // Bug report on this: https://svn.boost.org/trac/boost/ticket/11711
    SECTION("point NaN")
    {
        mapnik::geometry::point<double> pt(std::numeric_limits<double>::quiet_NaN(),
                                           std::numeric_limits<double>::quiet_NaN());
        CHECK(std::isnan(pt.x));
        CHECK(std::isnan(pt.y));
        CHECK(mapnik::geometry::is_simple(pt));
    }

    // This is funky that boost geometry is_simple does not check for infinity when dealing with a point
    // this test is here in case the logic ever changes
    // Bug report on this: https://svn.boost.org/trac/boost/ticket/11711
    SECTION("point Infinity")
    {
        mapnik::geometry::point<double> pt(std::numeric_limits<double>::infinity(),
                                           std::numeric_limits<double>::infinity());
        CHECK(std::isinf(pt.x));
        CHECK(std::isinf(pt.y));
        CHECK(mapnik::geometry::is_simple(pt));
    }

    SECTION("multi point")
    {
        mapnik::geometry::multi_point<double> mpt;
        mpt.emplace_back(0, 0);
        mpt.emplace_back(1, 1);
        CHECK(mapnik::geometry::is_simple(mpt));
    }

    SECTION("multi point empty")
    {
        mapnik::geometry::multi_point<double> mpt;
        CHECK(mapnik::geometry::is_simple(mpt));
    }

    SECTION("line_string")
    {
        mapnik::geometry::line_string<double> line;
        line.emplace_back(0, 0);
        line.emplace_back(1, 1);
        CHECK(mapnik::geometry::is_simple(line));
    }

    // This fails while is_valid will not fail!
    SECTION("line_string repeated points")
    {
        mapnik::geometry::line_string<double> line;
        line.emplace_back(0, 0);
        line.emplace_back(1, 1);
        line.emplace_back(1, 1);
        line.emplace_back(2, 2);
        CHECK(!mapnik::geometry::is_simple(line));
    }

    SECTION("line_string empty")
    {
        mapnik::geometry::line_string<double> line;
        CHECK(mapnik::geometry::is_simple(line));
    }

    SECTION("multi_line_string")
    {
        mapnik::geometry::line_string<double> line1;
        line1.emplace_back(0, 0);
        line1.emplace_back(1, 1);
        mapnik::geometry::line_string<double> line2;
        line2.emplace_back(0, 1);
        line2.emplace_back(1, 2);
        mapnik::geometry::multi_line_string<double> lines;
        lines.emplace_back(line1);
        lines.emplace_back(line2);
        CHECK(mapnik::geometry::is_simple(lines));
    }

    SECTION("multi_line_string empty")
    {
        mapnik::geometry::multi_line_string<double> lines;
        CHECK(mapnik::geometry::is_simple(lines));
    }

    SECTION("multi_line_string empty")
    {
        mapnik::geometry::multi_line_string<double> lines;
        mapnik::geometry::line_string<double> line;
        lines.emplace_back(line);
        CHECK(mapnik::geometry::is_simple(lines));
    }

    SECTION("polygon")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(1, 0);
        ring.emplace_back(1, 1);
        ring.emplace_back(0, 1);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        CHECK(mapnik::geometry::is_simple(poly));
    }

    SECTION("polygon invalid winding order")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(0, 1);
        ring.emplace_back(1, 1);
        ring.emplace_back(1, 0);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        CHECK(mapnik::geometry::is_simple(poly));
    }

    // repeated points are not considered invalid in a polygon
    // but they are considered not simple
    SECTION("polygon 2 repeated points")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(1, 0);
        ring.emplace_back(1, 1);
        ring.emplace_back(1, 1);
        ring.emplace_back(0, 1);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        CHECK(!mapnik::geometry::is_simple(poly));
    }
    // repeated points are not considered invalid in a polygon
    // but they are considered not simple
    SECTION("polygon 3 repeated points")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(1, 0);
        ring.emplace_back(1, 1);
        ring.emplace_back(1, 1);
        ring.emplace_back(1, 1);
        ring.emplace_back(0, 1);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        CHECK(!mapnik::geometry::is_simple(poly));
    }

#if BOOST_VERSION >= 106000

    SECTION("polygon that is empty")
    {
        mapnik::geometry::polygon<double> poly;
        poly.emplace_back();
        CHECK(!mapnik::geometry::is_simple(poly));
    }

    SECTION("polygon that has empty exterior ring")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        poly.push_back(std::move(ring));
        CHECK(!mapnik::geometry::is_simple(poly));
    }

    SECTION("polygon that has empty interior ring")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(1, 0);
        ring.emplace_back(1, 1);
        ring.emplace_back(0, 1);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        mapnik::geometry::linear_ring<double> ring2;
        poly.push_back(std::move(ring2));
        CHECK(!mapnik::geometry::is_simple(poly));
    }

#else // BOOST_VERSION >= 1.60

    SECTION("polygon that is empty")
    {
        mapnik::geometry::polygon<double> poly;
        CHECK(mapnik::geometry::is_simple(poly));
    }

    SECTION("polygon that has empty exterior ring")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        poly.push_back(std::move(ring));
        CHECK(mapnik::geometry::is_simple(poly));
    }

    SECTION("polygon that has empty interior ring")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(1, 0);
        ring.emplace_back(1, 1);
        ring.emplace_back(0, 1);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        mapnik::geometry::linear_ring<double> ring2;
        poly.push_back(std::move(ring2));
        CHECK(mapnik::geometry::is_simple(poly));
    }

#endif // BOOST_VERSION >= 1.60

    // A polygon with a spike can still be simple
    SECTION("polygon with spike")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(1, 0);
        ring.emplace_back(1, 1);
        ring.emplace_back(2, 2);
        ring.emplace_back(1, 1);
        ring.emplace_back(0, 1);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        CHECK(mapnik::geometry::is_simple(poly));
    }

    SECTION("polygon with hole")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(3, 0);
        ring.emplace_back(3, 3);
        ring.emplace_back(0, 3);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        mapnik::geometry::linear_ring<double> hole;
        hole.emplace_back(1, 1);
        hole.emplace_back(1, 2);
        hole.emplace_back(2, 2);
        hole.emplace_back(2, 1);
        hole.emplace_back(1, 1);
        poly.push_back(std::move(hole));
        CHECK(mapnik::geometry::is_simple(poly));
    }

    // Polygons with reversed winding order still can be considered simple
    SECTION("polygon with hole with invalid winding order")
    {
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(3, 0);
        ring.emplace_back(3, 3);
        ring.emplace_back(0, 3);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        mapnik::geometry::linear_ring<double> hole;
        hole.emplace_back(1, 1);
        hole.emplace_back(2, 1);
        hole.emplace_back(2, 2);
        hole.emplace_back(1, 2);
        hole.emplace_back(1, 1);
        poly.push_back(std::move(hole));
        CHECK(mapnik::geometry::is_simple(poly));
    }

    SECTION("multi polygon")
    {
        mapnik::geometry::multi_polygon<double> mp;
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;

        ring.emplace_back(0, 0);
        ring.emplace_back(1, 0);
        ring.emplace_back(1, 1);
        ring.emplace_back(0, 1);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        mapnik::geometry::polygon<double> poly2;
        mapnik::geometry::linear_ring<double> ring2;
        ring2.emplace_back(0, 0);
        ring2.emplace_back(-1, 0);
        ring2.emplace_back(-1, -1);
        ring2.emplace_back(0, -1);
        ring2.emplace_back(0, 0);
        poly2.push_back(std::move(ring2));
        mp.emplace_back(poly);
        mp.emplace_back(poly2);
        CHECK(mapnik::geometry::is_simple(mp));
    }

    SECTION("multi polygon with hole")
    {
        mapnik::geometry::multi_polygon<double> mp;
        mapnik::geometry::polygon<double> poly;
        mapnik::geometry::linear_ring<double> ring;
        ring.emplace_back(0, 0);
        ring.emplace_back(3, 0);
        ring.emplace_back(3, 3);
        ring.emplace_back(0, 3);
        ring.emplace_back(0, 0);
        poly.push_back(std::move(ring));
        mapnik::geometry::linear_ring<double> hole;
        hole.emplace_back(1, 1);
        hole.emplace_back(1, 2);
        hole.emplace_back(2, 2);
        hole.emplace_back(2, 1);
        hole.emplace_back(1, 1);
        poly.push_back(std::move(hole));
        mapnik::geometry::polygon<double> poly2;
        mapnik::geometry::linear_ring<double> ring2;
        ring2.emplace_back(0, 0);
        ring2.emplace_back(-3, 0);
        ring2.emplace_back(-3, -3);
        ring2.emplace_back(0, -3);
        ring2.emplace_back(0, 0);
        poly2.push_back(std::move(ring2));
        mapnik::geometry::linear_ring<double> hole2;
        hole2.emplace_back(-1, -1);
        hole2.emplace_back(-1, -2);
        hole2.emplace_back(-2, -2);
        hole2.emplace_back(-2, -1);
        hole2.emplace_back(-1, -1);
        poly2.push_back(std::move(hole2));
        mp.emplace_back(poly);
        mp.emplace_back(poly2);
        CHECK(mapnik::geometry::is_simple(mp));
    }

    SECTION("multi polygon empty")
    {
        mapnik::geometry::multi_polygon<double> mp;
        CHECK(mapnik::geometry::is_simple(mp));
    }

#else // BOOST_VERSION >= 1.58

    SECTION("skipped is_simple tests")
    {
        WARN("geometry simple tests disabled due to boost version older that 1.58 used");
    }

#endif
}
