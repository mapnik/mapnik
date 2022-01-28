#include "catch.hpp"

#include <mapnik/geometry.hpp>
#include <mapnik/hit_test_filter.hpp>
#include <mapnik/geometry/correct.hpp>

TEST_CASE("geometry ops")
{
    SECTION("hit_test_filter - double")
    {
        using namespace mapnik::geometry;
        {
            geometry<double> geom(point<double>(0, 0));
            REQUIRE(mapnik::hit_test(geom, 0, 0, 0));
        }
        {
            geometry<double> geom(point<double>(0, 0));
            REQUIRE(mapnik::hit_test(geom, 1, 0, 1));
        }
        {
            geometry<double> geom(point<double>(0, 0));
            REQUIRE(mapnik::hit_test(geom, 0, 1, 1));
        }
        {
            geometry<double> geom(point<double>(0, 0));
            REQUIRE(mapnik::hit_test(geom, 1, 1, 1.5));
        }
        {
            line_string<double> line;
            line.emplace_back(0, 0);
            line.emplace_back(1, 1);
            line.emplace_back(2, 2);
            geometry<double> geom(line);
            REQUIRE(mapnik::hit_test(geom, 0, 0, 1.5));
        }
        {
            line_string<double> line;
            line.emplace_back(0, 0);
            line.emplace_back(1, 1);
            line.emplace_back(2, 2);
            multi_line_string<double> multi_line;
            multi_line.emplace_back(std::move(line));
            geometry<double> geom(multi_line);
            REQUIRE(mapnik::hit_test(geom, 0, 0, 1.5));
        }
        {
            polygon<double> poly;
            linear_ring<double> ring;
            ring.emplace_back(0, 0);
            ring.emplace_back(-10, 0);
            ring.emplace_back(-10, 10);
            ring.emplace_back(0, 10);
            ring.emplace_back(0, 0);
            poly.push_back(std::move(ring));
            geometry<double> geom(poly);
            REQUIRE(mapnik::hit_test(geom, -5, 5, 0));

            multi_polygon<double> mp;
            mp.push_back(poly);
            geometry<double> geom_mp(mp);
            REQUIRE(mapnik::hit_test(geom_mp, -5, 5, 0));

            correct(geom);
            REQUIRE(mapnik::hit_test(geom, -5, 5, 0));
            correct(geom_mp);
            REQUIRE(mapnik::hit_test(geom_mp, -5, 5, 0));

            geometry_collection<double> gc;
            REQUIRE(!mapnik::hit_test(geometry<double>(gc), -5, 5, 0));
            gc.push_back(geom_mp);
            REQUIRE(mapnik::hit_test(geometry<double>(gc), -5, 5, 0));
            REQUIRE(!mapnik::hit_test(geometry<double>(gc), -50, -50, 0));
            gc.emplace_back(point<double>(-50, -50));
            REQUIRE(mapnik::hit_test(geometry<double>(gc), -50, -50, 0));
        }

        {
            // polygon with hole
            polygon<double> poly;
            linear_ring<double> ring;
            ring.emplace_back(0, 0);
            ring.emplace_back(-10, 0);
            ring.emplace_back(-10, 10);
            ring.emplace_back(0, 10);
            ring.emplace_back(0, 0);
            poly.push_back(std::move(ring));
            linear_ring<double> hole;
            hole.emplace_back(-7, 7);
            hole.emplace_back(-7, 3);
            hole.emplace_back(-3, 3);
            hole.emplace_back(-3, 7);
            hole.emplace_back(-7, 7);
            poly.push_back(std::move(hole));
            geometry<double> geom(poly);
            REQUIRE(!mapnik::hit_test(geom, -5, 5, 0));
            // add another hole inside the first hole
            // which should be considered a hit
            linear_ring<double> fill;
            fill.emplace_back(-6, 4);
            fill.emplace_back(-6, 6);
            fill.emplace_back(-4, 6);
            fill.emplace_back(-4, 4);
            fill.emplace_back(-6, 4);
            poly.push_back(std::move(fill));
            REQUIRE(mapnik::hit_test(geometry<double>(poly), -5, 5, 0));
        }
    }
}
