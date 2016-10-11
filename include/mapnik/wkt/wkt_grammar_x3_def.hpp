/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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

#ifndef MAPNIK_WKT_GRAMMAR_X3_DEF_HPP
#define MAPNIK_WKT_GRAMMAR_X3_DEF_HPP

#include <mapnik/wkt/wkt_grammar_x3.hpp>
#include <mapnik/geometry/fusion_adapted.hpp>

namespace mapnik { namespace grammar {

namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;
using x3::lit;
using x3::double_;
using x3::no_case;

// functors
auto make_empty = [](auto const& ctx)
{
    _val(ctx) = geometry::geometry_empty();
};

auto set_exterior = [](auto const& ctx)
{
    _val(ctx).set_exterior_ring(std::move(_attr(ctx)));
};

auto add_hole = [](auto const& ctx)
{
    _val(ctx).add_hole(std::move(_attr(ctx)));
};

// start rule
wkt_grammar_type const wkt("wkt");
// rules
x3::rule<class empty, mapnik::geometry::geometry_empty> const empty("EMPTY");
x3::rule<class point, mapnik::geometry::geometry<double> > const point("POINT");
x3::rule<class line_string, mapnik::geometry::line_string<double> > const line_string("LINESTRING");
x3::rule<class polygon, mapnik::geometry::polygon<double> > const polygon("POLYGON");
x3::rule<class multi_point, mapnik::geometry::multi_point<double> > const multi_point("MULTIPOINT");
x3::rule<class multi_line_string, mapnik::geometry::multi_line_string<double> > const multi_line_string("MULTILINESTRING");
x3::rule<class multi_polygon, mapnik::geometry::multi_polygon<double> > const multi_polygon("MULTIPOLYGON");
x3::rule<class geometry_collection, mapnik::geometry::geometry_collection<double> > const geometry_collection("GEOMETRYCOLLECTION");

x3::rule<class point_text, mapnik::geometry::point<double> > const point_text("point-text");
x3::rule<class positions, mapnik::geometry::line_string<double> > const positions("positions");
x3::rule<class polygon_rings, mapnik::geometry::polygon<double> > const polygon_rings("polygon-rings");
x3::rule<class points, mapnik::geometry::multi_point<double> > const points("points");
x3::rule<class lines, mapnik::geometry::multi_line_string<double> > const lines("lines");
x3::rule<class polygons, mapnik::geometry::multi_polygon<double> > const polygons("polygons");
x3::rule<class geometries, mapnik::geometry::geometry_collection<double> > const geometries("geometries");


auto const point_text_def = '(' > double_ > double_ > ')';
auto const positions_def = lit('(') > (double_ > double_) % lit(',') > lit(')');
auto const polygon_rings_def = '(' > positions[set_exterior] > *(lit(',') > positions[add_hole]) > ')';
auto const points_def = (lit('(') >> ((point_text_def % ',') > lit(')'))) | positions_def ;
auto const lines_def = lit('(') > (positions_def % lit(',')) > lit(')');
auto const polygons_def = lit('(') > (polygon_rings % lit(',')) > lit(')');
auto const geometries_def = lit('(') > -(wkt % ',') > lit(')');

//
auto const wkt_def = point | line_string | polygon | multi_point | multi_line_string | multi_polygon | geometry_collection;

// EMPTY
auto const empty_def = no_case["EMPTY"][make_empty];
// POINT
auto const point_def = no_case["POINT"] > (point_text | empty);
// LINESTRING
auto const line_string_def = no_case["LINESTRING"] > (positions | no_case["EMPTY"]);
// POLYGON
auto const polygon_def = no_case["POLYGON"] > (polygon_rings | no_case["EMPTY"]);
// MULTIPOINT
auto const multi_point_def = no_case["MULTIPOINT"] > (points | no_case["EMPTY"]);
// MULTILINESTRING
auto const multi_line_string_def = no_case["MULTILINESTRING"] > (lines | no_case["EMPTY"]);
// MULTIPOLYGON
auto const multi_polygon_def = no_case["MULTIPOLYGON"] > (polygons | no_case["EMPTY"]);
// GEOMETRYCOLLECTION
auto const geometry_collection_def = no_case["GEOMETRYCOLLECTION"] > (geometries | no_case["EMPTY"]);

BOOST_SPIRIT_DEFINE(
    wkt,
    point_text,
    positions,
    points,
    lines,
    polygons,
    geometries,
    empty,
    point,
    line_string,
    polygon_rings,
    polygon,
    multi_point,
    multi_line_string,
    multi_polygon,
    geometry_collection
    );
}}

namespace mapnik
{
grammar::wkt_grammar_type const& wkt_grammar()
{
    return grammar::wkt;
}
}

#endif // MAPNIK_WKT_GRAMMAR_X3_DEF_HPP
