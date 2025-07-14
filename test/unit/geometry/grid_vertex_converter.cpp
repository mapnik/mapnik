#include "catch.hpp"

#include <mapnik/grid_vertex_converter.hpp>

TEST_CASE("spiral_iterator")
{
    SECTION("sprial 3x3")
    {
        mapnik::geometry::spiral_iterator si(3);
        mapnik::geometry::point<int> const points[] =
          {{0, 0}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};

        std::size_t const points_size = std::extent<decltype(points)>::value;

        int x, y;
        std::size_t index = 0;

        while (si.vertex(&x, &y))
        {
            REQUIRE(index < points_size);

            CHECK(x == points[index].x);
            CHECK(y == points[index].y);

            index++;
        }

        CHECK(index == points_size);
    }
}

TEST_CASE("grid_vertex_converter")
{
    SECTION("empty polygon")
    {
        mapnik::geometry::polygon<double> poly;
        using path_type = mapnik::geometry::polygon_vertex_adapter<double>;
        path_type path(poly);
        using converter_type = mapnik::geometry::grid_vertex_converter<path_type, double>;
        converter_type gvc(path, 10.0, 10.0, 1.0);

        double x, y;
        unsigned cmd = gvc.vertex(&x, &y);

        CHECK(cmd == mapnik::SEG_END);
    }

    SECTION("empty polygon exterior ring")
    {
        mapnik::geometry::polygon<double> poly;
        poly.emplace_back();

        using path_type = mapnik::geometry::polygon_vertex_adapter<double>;
        path_type path(poly);
        using converter_type = mapnik::geometry::grid_vertex_converter<path_type, double>;
        converter_type gvc(path, 10.0, 10.0, 1.0);

        double x, y;
        unsigned cmd = gvc.vertex(&x, &y);

        CHECK(cmd == mapnik::SEG_END);
    }

    SECTION("grid of a square")
    {
        mapnik::geometry::polygon<double> poly;
        poly.emplace_back();
        auto& exterior_ring = poly.front();
        exterior_ring.emplace_back(-10, -10);
        exterior_ring.emplace_back(10, -10);
        exterior_ring.emplace_back(10, 10);
        exterior_ring.emplace_back(-10, 10);
        exterior_ring.emplace_back(-10, -10);

        using path_type = mapnik::geometry::polygon_vertex_adapter<double>;
        path_type path(poly);
        using converter_type = mapnik::geometry::grid_vertex_converter<path_type, double>;
        converter_type gvc(path, 3.0, 3.0, 1.0);

        mapnik::geometry::point<double> const points[] = {
          {0, 0},   {3, 0},   {3, 3},   {0, 3},   {-3, 3},  {-3, 0}, {-3, -3}, {0, -3}, {3, -3}, {6, -3},
          {6, 0},   {6, 3},   {6, 6},   {3, 6},   {0, 6},   {-3, 6}, {-6, 6},  {-6, 3}, {-6, 0}, {-6, -3},
          {-6, -6}, {-3, -6}, {0, -6},  {3, -6},  {6, -6},  {9, -6}, {9, -3},  {9, 0},  {9, 3},  {9, 6},
          {9, 9},   {6, 9},   {3, 9},   {0, 9},   {-3, 9},  {-6, 9}, {-9, 9},  {-9, 6}, {-9, 3}, {-9, 0},
          {-9, -3}, {-9, -6}, {-9, -9}, {-6, -9}, {-3, -9}, {0, -9}, {3, -9},  {6, -9}, {9, -9}};
        std::size_t const points_size = std::extent<decltype(points)>::value;

        double x, y;
        unsigned cmd = mapnik::SEG_END;
        std::size_t index = 0;

        while ((cmd = gvc.vertex(&x, &y)) != mapnik::SEG_END)
        {
            REQUIRE(index < points_size);

            CHECK(cmd == mapnik::SEG_MOVETO);
            CHECK(x == Approx(points[index].x));
            CHECK(y == Approx(points[index].y));

            index++;
        }

        CHECK(index == points_size);
        CHECK(cmd == mapnik::SEG_END);
    }
}
