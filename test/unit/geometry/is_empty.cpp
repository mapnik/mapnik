/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/


#include "catch.hpp"

#include <mapnik/geometry/is_empty.hpp>

TEST_CASE("geometry is_empty")
{
    SECTION("empty geometry")
    {
        mapnik::geometry::geometry_empty geom;
        REQUIRE(mapnik::geometry::is_empty(geom));
    }

    SECTION("geometry collection")
    {
        {
            mapnik::geometry::geometry_collection<double> geom;
            REQUIRE(mapnik::geometry::is_empty(geom));
        }
        {
            mapnik::geometry::geometry_collection<double> geom;
            mapnik::geometry::geometry_empty geom1;
            geom.emplace_back(std::move(geom1));
            REQUIRE(!mapnik::geometry::is_empty(geom));
        }
    }

    SECTION("point")
    {
        mapnik::geometry::point<double> pt(10, 10);
        REQUIRE(!mapnik::geometry::is_empty(pt));
    }

    SECTION("linestring")
    {
        {
            mapnik::geometry::line_string<double> line;
            REQUIRE(mapnik::geometry::is_empty(line));
        }
        {
            mapnik::geometry::line_string<double> line;
            line.emplace_back(0, 0);
            line.emplace_back(25, 25);
            line.emplace_back(50, 50);
            REQUIRE(!mapnik::geometry::is_empty(line));
        }
    }

    SECTION("polygon")
    {
        {
            mapnik::geometry::polygon<double> poly;
            REQUIRE(mapnik::geometry::is_empty(poly));
        }
        {
            mapnik::geometry::polygon<double> poly;
            mapnik::geometry::linear_ring<double> ring;
            poly.push_back(std::move(ring));
            REQUIRE(mapnik::geometry::is_empty(poly));
        }
        {
            mapnik::geometry::polygon<double> poly;
            mapnik::geometry::linear_ring<double> ring;
            ring.emplace_back(0, 0);
            ring.emplace_back(1, 0);
            ring.emplace_back(1, 1);
            ring.emplace_back(0, 1);
            ring.emplace_back(0, 0);
            poly.push_back(std::move(ring));
            REQUIRE(!mapnik::geometry::is_empty(poly));
        }
    }

    SECTION("multi-point")
    {
        {
            mapnik::geometry::multi_point<double> geom;
            REQUIRE(mapnik::geometry::is_empty(geom));
        }
        {
            mapnik::geometry::multi_point<double> geom;
            geom.emplace_back(0, 0);
            geom.emplace_back(25, 25);
            geom.emplace_back(50, 50);
            REQUIRE(!mapnik::geometry::is_empty(geom));
        }
    }

    SECTION("multi-linestring")
    {
        {
            mapnik::geometry::multi_line_string<double> geom;
            REQUIRE(mapnik::geometry::is_empty(geom));
        }
        {
            mapnik::geometry::multi_line_string<double> geom;
            mapnik::geometry::line_string<double> line;
            geom.emplace_back(std::move(line));
            REQUIRE(!mapnik::geometry::is_empty(geom));
        }
    }

    SECTION("multi-polygon")
    {
        {
            mapnik::geometry::multi_polygon<double> geom;
            REQUIRE(mapnik::geometry::is_empty(geom));
        }
        {
            mapnik::geometry::multi_polygon<double> geom;
            mapnik::geometry::polygon<double> poly;
            mapnik::geometry::linear_ring<double> ring;
            poly.push_back(std::move(ring));
            geom.emplace_back(std::move(poly));
            REQUIRE(!mapnik::geometry::is_empty(geom));
        }
    }
}
