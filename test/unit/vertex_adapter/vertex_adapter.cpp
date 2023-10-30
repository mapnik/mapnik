#include "catch.hpp"

#include <mapnik/vertex_adapters.hpp>
#include <mapnik/geometry/correct.hpp>

TEST_CASE("vertex_adapters")
{
    SECTION("polygon")
    {
        mapnik::geometry::polygon<double> g;
        g.emplace_back();
        g.back().emplace_back(1, 1);
        g.back().emplace_back(2, 2);
        g.back().emplace_back(100, 100);
        g.back().emplace_back(1, 1);

        mapnik::geometry::polygon_vertex_adapter<double> va(g);
        double x, y;
        unsigned cmd;

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_MOVETO);
        REQUIRE(x == 1);
        REQUIRE(y == 1);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == 2);
        REQUIRE(y == 2);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == 100);
        REQUIRE(y == 100);

        // close
        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_CLOSE);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        // end
        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_END);
        REQUIRE(x == 0);
        REQUIRE(y == 0);
    }

    SECTION("polygon with hole")
    {
        mapnik::geometry::polygon<double> g;
        g.emplace_back();
        g.back().emplace_back(0, 0);
        g.back().emplace_back(-10, 0);
        g.back().emplace_back(-10, 10);
        g.back().emplace_back(0, 10);
        g.back().emplace_back(0, 0);
        std::vector<mapnik::geometry::linear_ring<double>> interior_rings;
        mapnik::geometry::linear_ring<double> hole;
        hole.emplace_back(-7, 7);
        hole.emplace_back(-7, 3);
        hole.emplace_back(-3, 3);
        hole.emplace_back(-3, 7);
        hole.emplace_back(-7, 7);
        g.push_back(std::move(hole));

        mapnik::geometry::linear_ring<double> hole_in_hole;
        hole_in_hole.emplace_back(-6, 4);
        hole_in_hole.emplace_back(-6, 6);
        hole_in_hole.emplace_back(-4, 6);
        hole_in_hole.emplace_back(-4, 4);
        hole_in_hole.emplace_back(-6, 4);
        g.push_back(std::move(hole_in_hole));

        mapnik::geometry::polygon_vertex_adapter<double> va(g);
        double x, y;
        unsigned cmd;

        // exterior ring
        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_MOVETO);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -10);
        REQUIRE(y == 0);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -10);
        REQUIRE(y == 10);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == 0);
        REQUIRE(y == 10);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_CLOSE);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        // exterior ring via ring_vertex_adapter
        mapnik::geometry::ring_vertex_adapter<double> va2(g.front());
        cmd = va2.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_MOVETO);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        cmd = va2.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -10);
        REQUIRE(y == 0);

        cmd = va2.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -10);
        REQUIRE(y == 10);

        cmd = va2.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == 0);
        REQUIRE(y == 10);

        cmd = va2.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_CLOSE);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        // since ring adapter is only for exterior, next should be END
        cmd = va2.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_END);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        // first hole for polygon_adapter
        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_MOVETO);
        REQUIRE(x == -7);
        REQUIRE(y == 7);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -7);
        REQUIRE(y == 3);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -3);
        REQUIRE(y == 3);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -3);
        REQUIRE(y == 7);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_CLOSE);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        // second hole
        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_MOVETO);
        REQUIRE(x == -6);
        REQUIRE(y == 4);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -6);
        REQUIRE(y == 6);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -4);
        REQUIRE(y == 6);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -4);
        REQUIRE(y == 4);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_CLOSE);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        mapnik::geometry::correct(g);

        va.rewind(0);

        // exterior ring: flipped winding order from correct
        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_MOVETO);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == 0);
        REQUIRE(y == 10);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -10);
        REQUIRE(y == 10);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -10);
        REQUIRE(y == 0);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_CLOSE);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        // first hole: flipped winding order from correct
        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_MOVETO);
        REQUIRE(x == -7);
        REQUIRE(y == 7);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -3);
        REQUIRE(y == 7);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -3);
        REQUIRE(y == 3);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -7);
        REQUIRE(y == 3);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_CLOSE);
        REQUIRE(x == 0);
        REQUIRE(y == 0);

        // second hole: correct appears not to have changed winding order
        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_MOVETO);
        REQUIRE(x == -6);
        REQUIRE(y == 4);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -6);
        REQUIRE(y == 6);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -4);
        REQUIRE(y == 6);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_LINETO);
        REQUIRE(x == -4);
        REQUIRE(y == 4);

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_CLOSE);
        REQUIRE(x == 0);
        REQUIRE(y == 0);
    }

    SECTION("polygon with empty exterior ring")
    {
        mapnik::geometry::polygon<double> g;
        g.emplace_back();

        mapnik::geometry::polygon_vertex_adapter<double> va(g);
        double x = 0, y = 0;
        unsigned cmd;

        cmd = va.vertex(&x, &y);
        CHECK(cmd == mapnik::SEG_END);
        CHECK(x == Approx(0));
        CHECK(y == Approx(0));
    }

    SECTION("polygon with empty interior ring")
    {
        mapnik::geometry::polygon<double> g;
        g.emplace_back();
        g.back().emplace_back(-1, -1);
        g.back().emplace_back(1, -1);
        g.back().emplace_back(1, 1);
        g.back().emplace_back(-1, 1);
        g.back().emplace_back(-1, -1);

        // Emplace empty interior ring
        g.emplace_back();

        mapnik::geometry::polygon_vertex_adapter<double> va(g);
        double x, y;
        unsigned cmd;

        cmd = va.vertex(&x, &y);
        CHECK(cmd == mapnik::SEG_MOVETO);
        CHECK(x == Approx(-1));
        CHECK(y == Approx(-1));

        cmd = va.vertex(&x, &y);
        CHECK(cmd == mapnik::SEG_LINETO);
        CHECK(x == Approx(1));
        CHECK(y == Approx(-1));

        cmd = va.vertex(&x, &y);
        CHECK(cmd == mapnik::SEG_LINETO);
        CHECK(x == Approx(1));
        CHECK(y == Approx(1));

        cmd = va.vertex(&x, &y);
        CHECK(cmd == mapnik::SEG_LINETO);
        CHECK(x == Approx(-1));
        CHECK(y == Approx(1));

        cmd = va.vertex(&x, &y);
        CHECK(cmd == mapnik::SEG_CLOSE);
        CHECK(x == Approx(0));
        CHECK(y == Approx(0));

        cmd = va.vertex(&x, &y);
        CHECK(cmd == mapnik::SEG_CLOSE);
        CHECK(x == Approx(0));
        CHECK(y == Approx(0));

        cmd = va.vertex(&x, &y);
        REQUIRE(cmd == mapnik::SEG_END);
        REQUIRE(x == Approx(0));
        REQUIRE(y == Approx(0));
    }
}
