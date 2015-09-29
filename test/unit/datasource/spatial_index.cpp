/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#include <mapnik/util/spatial_index.hpp>
#include <mapnik/quad_tree.hpp>

TEST_CASE("spatial_index")
{
    SECTION("mapnik::quad_tree<T>")
    {
        // value type to store inside index (must provide operator<< + operator>>)
        using value_type = std::size_t;
        mapnik::box2d<double> extent(0,0,100,100);
        mapnik::quad_tree<value_type> tree(extent);
        REQUIRE(tree.extent() == extent);
        // insert some items
        tree.insert(1, mapnik::box2d<double>(10,10,20,20));
        tree.insert(2, mapnik::box2d<double>(30,30,40,40));
        tree.insert(3, mapnik::box2d<double>(30,10,40,20));
        REQUIRE(tree.count() == 3);
        REQUIRE(tree.count_items() == 3);
    }
}
