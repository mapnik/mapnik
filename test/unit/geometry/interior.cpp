#include "catch.hpp"

#include <mapnik/vertex_adapters.hpp>
#include <mapnik/geometry/interior.hpp>

TEST_CASE("interior") {

SECTION("bisector") {

    using bisector_type = mapnik::geometry::detail::bisector;
    using point_type = mapnik::geometry::detail::point_type;

    {
        bisector_type bisector(point_type(0.5, 0.5), 0);
        CHECK(!bisector.intersects(point_type(0, 0), point_type(1, 0)));
        CHECK(bisector.intersects(point_type(0, 0), point_type(1, 1)));
    }

    {
        bisector_type bisector(point_type(270.20886059972594, 253.57572393803318), 3.0 * M_PI / 4.0);
        const point_type p1(179.15547826136165, 399.62713043390545);
        const point_type p2(151.90817391149596, 364.54399999915978);

        CHECK(bisector.intersects(p1, p2));
        CHECK(bisector.intersects(p2, p1));

        const point_type intersection = bisector.intersection(p1, p2);
        CHECK(intersection.x == Approx(155.1134848429));
        CHECK(intersection.y == Approx(368.6710996948));
    }

    {
        bisector_type bisector(point_type(0.5, 0.5), M_PI / 4.0);
        const point_type point(1, 1);
        const double distance = bisector.distance_to_center(point);
        CHECK(distance == Approx(1.0 / std::sqrt(2)));
    }

    {
        bisector_type bisector(point_type(0.5, 0.5), 3.0 * M_PI / 4.0);
        const point_type point(-1, 1);
        const double distance = bisector.distance_to_center(point);
        CHECK(distance == Approx(std::sqrt(2)));
    }
}

SECTION("bisector crosses vertex") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(1, 0);
    ring.emplace_back(0, 1);
    ring.emplace_back(-1, 0);
    ring.emplace_back(0, -1);
    ring.emplace_back(1, 0);
    poly.push_back(std::move(ring));

    mapnik::geometry::polygon_vertex_adapter<double> va(poly);

    using intersector_type = mapnik::geometry::detail::intersector;
    using intersection_type = mapnik::geometry::detail::intersection;
    using point_type = mapnik::geometry::detail::point_type;

    {
        const point_type center(0, 0);
        intersector_type ir(center, 1);
        ir.apply(va);

        REQUIRE(ir.intersections_per_bisector.size() == 1);
        REQUIRE(ir.bisectors.size() == 1);
        CHECK(!ir.bisectors[0]);

        std::vector<intersection_type> const& intersections = ir.intersections_per_bisector.front();

        CHECK(intersections.size() == 0);
    }

    {
        const point_type center(0, 1);
        intersector_type ir(center, 1);
        ir.apply(va);

        REQUIRE(ir.intersections_per_bisector.size() == 1);
        REQUIRE(ir.bisectors.size() == 1);
        CHECK(!ir.bisectors[0]);

        std::vector<intersection_type> const& intersections = ir.intersections_per_bisector.front();

        CHECK(intersections.size() == 0);
    }
}

SECTION("segment parallel to bisector") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0, 0);
    ring.emplace_back(1, 0);
    ring.emplace_back(1, 1);
    ring.emplace_back(0, 1);
    ring.emplace_back(0, 0);
    poly.push_back(std::move(ring));

    mapnik::geometry::polygon_vertex_adapter<double> va(poly);

    using intersector_type = mapnik::geometry::detail::intersector;
    using intersection_type = mapnik::geometry::detail::intersection;
    using point_type = mapnik::geometry::detail::point_type;

    const point_type center(0.5, 1);
    intersector_type ir(center, 1);
    ir.apply(va);

    REQUIRE(ir.intersections_per_bisector.size() == 1);
    REQUIRE(ir.bisectors.size() == 1);
    CHECK(!ir.bisectors[0]);

    std::vector<intersection_type> const& intersections = ir.intersections_per_bisector.front();

    CHECK(intersections.size() == 0);
}

SECTION("polygon - a square") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(0, 0);
    ring.emplace_back(1, 0);
    ring.emplace_back(1, 1);
    ring.emplace_back(0, 1);
    ring.emplace_back(0, 0);
    poly.push_back(std::move(ring));

    mapnik::geometry::polygon_vertex_adapter<double> va(poly);

    mapnik::geometry::point<double> interior;
    REQUIRE(mapnik::geometry::interior(va, interior.x, interior.y));
    CHECK(interior.x == 0.5);
    CHECK(interior.y == 0.5);
}

SECTION("polygon - bisectors crosses vertices") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(1, 0);
    ring.emplace_back(0, 1);
    ring.emplace_back(-1, 0);
    ring.emplace_back(0, -1);
    ring.emplace_back(1, 0);
    poly.push_back(std::move(ring));

    mapnik::geometry::polygon_vertex_adapter<double> va(poly);

    mapnik::geometry::point<double> interior;
    REQUIRE(mapnik::geometry::interior(va, interior.x, interior.y));
    CHECK(interior.x == 0.0);
    CHECK(interior.y == 0.0);
}

SECTION("polygon - bisectors crosses vertices, only horizontal bisector") {

    mapnik::geometry::polygon<double> poly;
    mapnik::geometry::linear_ring<double> ring;
    ring.emplace_back(1, 0);
    ring.emplace_back(0, 1);
    ring.emplace_back(-1, 0);
    ring.emplace_back(0, -1);
    ring.emplace_back(1, 0);
    poly.push_back(std::move(ring));

    mapnik::geometry::polygon_vertex_adapter<double> va(poly);

    mapnik::geometry::point<double> interior;
    REQUIRE(mapnik::geometry::interior(va, interior.x, interior.y, 1));
    CHECK(interior.x == 0.0);
    CHECK(interior.y == 0.0);
}

}
