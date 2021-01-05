/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#include <mapnik/quad_tree.hpp>
#include <mapnik/util/spatial_index.hpp>

TEST_CASE("spatial_index")
{
    SECTION("mapnik::quad_tree<T>")
    {
        // value_type must have standard layout (http://en.cppreference.com/w/cpp/types/is_standard_layout)
        using value_type = std::int32_t;
        using mapnik::filter_in_box;
        mapnik::box2d<double> extent(0,0,100,100);
        mapnik::quad_tree<value_type> tree(extent);
        REQUIRE(tree.extent() == extent);
        // insert some items
        tree.insert(1, mapnik::box2d<double>(10,10,20,20));
        tree.insert(2, mapnik::box2d<double>(30,30,40,40));
        tree.insert(3, mapnik::box2d<double>(30,10,40,20));
        tree.insert(4, mapnik::box2d<double>(1,1,2,2));
        tree.trim();

        REQUIRE(tree.count() == 5);
        REQUIRE(tree.count_items() == 4);

        // serialise
        std::ostringstream out(std::ios::binary);
        tree.write(out);
        out.flush();

        REQUIRE(out.str().length() == 252);
        REQUIRE(out.str().at(0) == 'm');

        // read bounding box
        std::istringstream in(out.str(), std::ios::binary);
        auto box = mapnik::util::spatial_index<value_type, filter_in_box, std::istringstream>::bounding_box(in);
        REQUIRE(box == tree.extent());
        // bounding box query
        std::vector<value_type> results;
        filter_in_box filter(box);
        mapnik::util::spatial_index<value_type, filter_in_box, std::istringstream>::query(filter, in, results);

        REQUIRE(results[0] == 1);
        REQUIRE(results[1] == 4);
        REQUIRE(results[2] == 3);
        REQUIRE(results[3] == 2);
        REQUIRE(results.size() == 4);

        // query first N elements interface
        results.clear();
        in.seekg(0, std::ios::beg);
        mapnik::util::spatial_index<value_type, filter_in_box, std::istringstream>::query_first_n(filter, in, results, 2);
        REQUIRE(results.size() == 2);
        REQUIRE(results[0] == 1);
        REQUIRE(results[1] == 4);
        results.clear();
        in.seekg(0, std::ios::beg);
        mapnik::util::spatial_index<value_type, filter_in_box, std::istringstream>::query_first_n(filter, in, results, 5);
        REQUIRE(results[0] == 1);
        REQUIRE(results[1] == 4);
        REQUIRE(results[2] == 3);
        REQUIRE(results[3] == 2);
        REQUIRE(results.size() == 4);
    }
}
