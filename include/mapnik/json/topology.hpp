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

#ifndef MAPNIK_TOPOLOGY_HPP
#define MAPNIK_TOPOLOGY_HPP

#include <mapnik/json/generic_json.hpp>
#include <mapnik/util/variant.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/optional.hpp>
#pragma GCC diagnostic pop

#include <tuple>
#include <vector>
#include <list>

namespace mapnik { namespace topojson {

using index_type = int;

struct coordinate
{
    double x;
    double y;
};

using property = std::tuple<std::string, json::json_value >;
using properties = std::vector<property>;

struct point
{
    coordinate coord;
    boost::optional<properties> props;
};

struct multi_point
{
    std::vector<coordinate> points;
    boost::optional<properties> props;
};

struct linestring
{
    index_type ring ;
    boost::optional<properties> props;
};

struct multi_linestring
{
    std::vector<index_type> rings;
    boost::optional<properties> props;
};

struct polygon
{
    std::vector<std::vector<index_type> > rings;
    boost::optional<properties> props;
};

struct multi_polygon
{
    std::vector<std::vector<std::vector<index_type> > > polygons;
    boost::optional<properties> props;
};

struct invalid {};

using geometry =  util::variant<invalid,
                                point,
                                linestring,
                                polygon,
                                multi_point,
                                multi_linestring,
                                multi_polygon>;

using pair_type = std::tuple<double,double>;

struct arc
{
    std::list<coordinate> coordinates;
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

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::coordinate,
    (double, x)
    (double, y)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::arc,
    (std::list<mapnik::topojson::coordinate>, coordinates)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::transform,
    (double, scale_x)
    (double, scale_y)
    (double, translate_x)
    (double, translate_y)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::bounding_box,
    (double, minx)
    (double, miny)
    (double, maxx)
    (double, maxy)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::point,
    (mapnik::topojson::coordinate, coord)
    (boost::optional<mapnik::topojson::properties>, props)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::multi_point,
    (std::vector<mapnik::topojson::coordinate>, points)
    (boost::optional<mapnik::topojson::properties>, props)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::linestring,
    (mapnik::topojson::index_type, ring)
    (boost::optional<mapnik::topojson::properties>, props)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::multi_linestring,
    (std::vector<mapnik::topojson::index_type>, rings)
    (boost::optional<mapnik::topojson::properties>, props)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::polygon,
    (std::vector<std::vector<mapnik::topojson::index_type> >, rings)
    (boost::optional<mapnik::topojson::properties>, props)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::multi_polygon,
    (std::vector<std::vector<std::vector<mapnik::topojson::index_type> > >, polygons)
    (boost::optional<mapnik::topojson::properties>, props)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::topology,
    (std::vector<mapnik::topojson::geometry>, geometries)
    (std::vector<mapnik::topojson::arc>, arcs)
    (boost::optional<mapnik::topojson::transform>, tr)
    (boost::optional<mapnik::topojson::bounding_box>, bbox)
   )

#endif // MAPNIK_TOPOLOGY_HPP
