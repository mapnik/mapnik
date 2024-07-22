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

#include <mapnik/map.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/featureset.hpp>
#include <mapnik/load_map.hpp>

namespace {

bool test_query_point(mapnik::Map const& map,
                      double x,
                      double y,
                      std::string const& name,
                      std::string const& expected_val)
{
    auto featureset = map.query_map_point(0u, x, y);
    while (auto feature = featureset->next())
    {
        auto val = feature->get(name);
        return (val.to_string() == expected_val);
    }
    return false;
}
} // namespace

TEST_CASE("Query map point")
{
    SECTION("Polygons")
    {
        mapnik::Map map(882, 780);
        mapnik::load_map(map, "./test/data/good_maps/wgs842merc_reprojection.xml");
        map.zoom_all();
        CHECK(test_query_point(map, 351, 94, "ADMIN", "Greenland"));
        CHECK(test_query_point(map, 402, 182, "ADMIN", "Iceland"));
        CHECK(test_query_point(map, 339, 687, "ADMIN", "Antarctica"));
        CHECK(test_query_point(map, 35, 141, "ADMIN", "Russia"));
        CHECK(test_query_point(map, 737, 297, "ADMIN", "Japan"));
    }
}
