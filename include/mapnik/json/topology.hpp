/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_JSON_TOPOLOGY_HPP
#define MAPNIK_JSON_TOPOLOGY_HPP

#include <mapnik/json/json_value.hpp>
#include <mapnik/util/variant.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
#pragma GCC diagnostic pop

#include <tuple>
#include <vector>

namespace mapnik { namespace topojson {

using index_type = int;
using index_array = std::vector<index_type>;
using index_array2 = std::vector<index_array>;
using index_array3 = std::vector<index_array2>;

struct coordinate
{
    double x;
    double y;
};

using position_array = std::vector<coordinate>;

using property = std::tuple<std::string, json::json_value>;
using properties = std::vector<property>;

struct point
{
    coordinate coord;
    boost::optional<properties> props;
};

struct multi_point
{
    position_array points;
    boost::optional<properties> props;
};

struct linestring
{
    index_array arcs;
    boost::optional<properties> props;
};

struct multi_linestring
{
    index_array2 lines;
    boost::optional<properties> props;
};

struct polygon
{
    index_array2 rings;
    boost::optional<properties> props;
};

struct multi_polygon
{
    index_array3 polygons;
    boost::optional<properties> props;
};

struct empty {};

using geometry =  util::variant<empty,
                                point,
                                linestring,
                                polygon,
                                multi_point,
                                multi_linestring,
                                multi_polygon>;

using pair_type = std::tuple<double,double>;

struct arc
{
    position_array coordinates;
};

struct transform
{
    double scale_x;
    double scale_y;
    double translate_x;
    double translate_y;
};

struct bounding_box
{
    double minx;
    double miny;
    double maxx;
    double maxy;
};

struct topology
{
    std::vector<geometry> geometries;
    std::vector<arc> arcs;
    boost::optional<transform> tr;
    boost::optional<bounding_box> bbox;
};

}}

#endif // MAPNIK_JSON_TOPOLOGY_HPP
