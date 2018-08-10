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

#ifndef MAPNIK_JSON_TOPOJSON_GRAMMAR_X3_DEF_HPP
#define MAPNIK_JSON_TOPOJSON_GRAMMAR_X3_DEF_HPP

#include <mapnik/json/unicode_string_grammar_x3.hpp>
#include <mapnik/json/generic_json_grammar_x3.hpp>
#include <mapnik/json/topojson_grammar_x3.hpp>
#include <mapnik/init_priority.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/adapted/struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#pragma GCC diagnostic pop

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::coordinate,
    (double, x)
    (double, y)
    )

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::topojson::arc,
    (mapnik::topojson::position_array, coordinates)
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

namespace mapnik { namespace topojson { namespace grammar {

struct create_point
{
    using result_type = mapnik::topojson::point;
    template <typename T0, typename T1>
    result_type operator()(T0 & coord, T1 & props) const
    {
        mapnik::topojson::point pt;
        if (coord.template is<mapnik::topojson::coordinate>())
        {
            auto const& coord_ = coord.template get<mapnik::topojson::coordinate>();
            pt.coord = coord_;
            pt.props = props;
        }
        return pt;
    }
};

struct create_multi_point
{
    using result_type = mapnik::topojson::multi_point;
    template <typename T0, typename T1>
    result_type operator()(T0 & coords, T1 & props) const
    {
        mapnik::topojson::multi_point mpt;
        if (coords.template is<topojson::position_array>())
        {
            auto const& points = coords.template get<topojson::position_array>();
            mpt.points = points;
            mpt.props = props;
        }
        return mpt;
    }
};

struct create_line_string
{
    using result_type = mapnik::topojson::linestring;
    template <typename T0, typename T1>
    result_type operator()(T0 & arcs, T1 & props) const
    {
        mapnik::topojson::linestring line;
        if (arcs.template is<topojson::index_array>())
        {
            auto const& arcs_ = arcs.template get<topojson::index_array>();
            line.arcs = arcs_;
            line.props = props;
        }
        return line;
    }
};

struct create_multi_line_string
{
    using result_type = mapnik::topojson::multi_linestring;
    template <typename T0, typename T1>
    result_type operator()(T0 & arcs, T1 & props) const
    {
        mapnik::topojson::multi_linestring mline;
        if (arcs.template is<topojson::index_array2>())
        {
            auto const& arcs_ = arcs.template get<topojson::index_array2>();
            mline.lines = arcs_;
            mline.props = props;
        }
        return mline;
    }
};

struct create_polygon
{
    using result_type = mapnik::topojson::polygon;
    template <typename T0, typename T1>
    result_type operator()(T0 & arcs, T1 & props) const
    {
        mapnik::topojson::polygon poly;
        if (arcs.template is<topojson::index_array2>())
        {
            auto const& arcs_ = arcs.template get<topojson::index_array2>();
            poly.rings = arcs_;
            poly.props = props;
        }
        return poly;
    }
};

struct create_multi_polygon
{
    using result_type = mapnik::topojson::multi_polygon;
    template <typename T0, typename T1>
    result_type operator()(T0 & arcs, T1 & props) const
    {
        mapnik::topojson::multi_polygon mpoly;
        if (arcs.template is<topojson::index_array3>())
        {
            auto const& arcs_ = arcs.template get<topojson::index_array3>();
            mpoly.polygons = arcs_;
            mpoly.props = props;
        }
        return mpoly;
    }
};


auto create_geometry = [] (auto const& ctx)
{
    auto const geom_type = std::get<0>(_attr(ctx));
    auto const& coord = std::get<1>(_attr(ctx));
    auto const& arcs = std::get<2>(_attr(ctx));
    auto const& props = std::get<3>(_attr(ctx));
    mapnik::topojson::geometry geom; //empty
    switch (geom_type)
    {
    case 1: //Point
        geom = create_point()(coord, props);
        break;
    case 2: //LineString
        geom = create_line_string()(arcs, props);
        break;
    case 3: //Polygon
        geom = create_polygon()(arcs, props);
        break;
    case 4: //MultiPoint
        geom = create_multi_point()(coord, props);
        break;
    case 5: //MultiLineString
        geom = create_multi_line_string()(arcs, props);
        break;
    case 6: //MultiPolygon
        geom = create_multi_polygon()(arcs, props);
        break;
    }
    _val(ctx) = std::move(geom);
};


auto assign_bbox = [] (auto const& ctx)
{
    _val(ctx).bbox = std::move(_attr(ctx));
};

auto assign_transform = [] (auto const& ctx)
{
    _val(ctx).tr = std::move(_attr(ctx));
};

auto assign_arcs = [] (auto const& ctx)
{
    _val(ctx).arcs = std::move(_attr(ctx));
};

auto assign_objects = [] (auto const& ctx)
{
    _val(ctx).geometries = std::move(_attr(ctx));
};


auto push_geometry = [] (auto const& ctx)
{
    _val(ctx).push_back(std::move(_attr(ctx)));
};

auto push_collection = [] (auto const& ctx)
{
    auto & dest = _val(ctx);
    auto & src = _attr(ctx);
    if (dest.empty()) dest = std::move(src);
    else
    {
        dest.reserve(dest.size() + src.size());
        dest.insert(std::end(dest),
                    std::make_move_iterator(std::begin(src)),
                    std::make_move_iterator(std::end(src)));
    }
};


auto assign_geometry_type = [] (auto const& ctx)
{
    std::get<0>(_val(ctx)) = _attr(ctx);
};

auto assign_coordinates = [] (auto const& ctx)
{
    std::get<1>(_val(ctx)) = std::move(_attr(ctx));
};

auto assign_rings = [] (auto const& ctx)
{
    std::get<2>(_val(ctx)) = std::move(_attr(ctx));
};

auto assign_properties = [] (auto const& ctx)
{
    std::get<3>(_val(ctx)) = std::move(_attr(ctx));
};

auto assign_prop_name = [] (auto const& ctx)
{
    std::get<0>(_val(ctx)) = std::move(_attr(ctx));
};

auto assign_prop_value = [] (auto const& ctx)
{
    std::get<1>(_val(ctx)) = std::move(_attr(ctx));
};

namespace x3 = boost::spirit::x3;

using x3::lit;
using x3::double_;
using x3::int_;
using x3::omit;

namespace
{
// import unicode string rule
auto const& json_string = json::unicode_string_grammar();
// json value
auto const& json_value = json::generic_json_grammar();
}

using coordinates_type = util::variant<topojson::coordinate,
                                       topojson::position_array>;

using arcs_type = util::variant<topojson::empty,
                                topojson::index_array,
                                topojson::index_array2,
                                topojson::index_array3>;

struct topojson_geometry_type_ : x3::symbols<int>
{
    topojson_geometry_type_()
        : x3::symbols<int>(std::string("Geometry type"))
    {
        add
            ("\"Point\"",1)
            ("\"LineString\"",2)
            ("\"Polygon\"",3)
            ("\"MultiPoint\"",4)
            ("\"MultiLineString\"",5)
            ("\"MultiPolygon\"",6)
            ;
    }
} const topojson_geometry_type = {};

MAPNIK_SPIRIT_LOCAL_RULE(transform, topojson::transform)
    = lit("\"transform\"") > ':' > '{'
    > lit("\"scale\"") > lit(':')
    > lit('[')
    > double_ > lit(',')
    > double_ > lit(']') > lit(',')
    > lit("\"translate\"") > lit(':')
    > lit('[') > double_ > lit(',') > double_ > lit(']')
    > lit('}')
    ;

MAPNIK_SPIRIT_LOCAL_RULE(bbox, topojson::bounding_box)
    = lit("\"bbox\"") > ':'
    > lit('[') > double_ > lit(',') > double_
    > lit(',') > double_ > lit(',') > double_
    > lit(']')
    ;

MAPNIK_SPIRIT_LOCAL_RULE(position, topojson::coordinate)
    = '['
    > double_ > ',' > double_
    > *(',' > omit[double_])
    > ']'
    ;

// A MultiPoint can have no points.
MAPNIK_SPIRIT_LOCAL_RULE(positions, topojson::position_array)
    = '[' >> -(position % ',') >> ']'
    ;

// A MultiLineString or MultiPolygon can have no components.
MAPNIK_SPIRIT_LOCAL_RULE(empty_array, topojson::empty)
    = lit('[') >> ']'
    ;

MAPNIK_SPIRIT_LOCAL_RULE(line_indices, topojson::index_array)
    = '[' >> (int_ % ',') >> ']'
    ;

MAPNIK_SPIRIT_LOCAL_RULE(poly_indices, topojson::index_array2)
    = '[' >> (line_indices % ',') >> ']'
    ;

MAPNIK_SPIRIT_LOCAL_RULE(mpoly_indices, topojson::index_array3)
    = '[' >> (poly_indices % ',') >> ']'
    ;

MAPNIK_SPIRIT_LOCAL_RULE(rings_array, arcs_type)
    = empty_array
    | line_indices
    | poly_indices
    | mpoly_indices
    ;

MAPNIK_SPIRIT_LOCAL_RULE(property, topojson::property)
    = json_string[assign_prop_name] > ':' > json_value[assign_prop_value]
    ;

MAPNIK_SPIRIT_LOCAL_RULE(properties, topojson::properties)
    = '{' > -(property % ',') > '}'
    ;

MAPNIK_SPIRIT_LOCAL_RULE(geometry_tuple,
                         std::tuple<int,
                                    coordinates_type,
                                    arcs_type,
                                    topojson::properties>)
    =
    ((lit("\"type\"") > lit(':') > topojson_geometry_type[assign_geometry_type])
     |
     (lit("\"coordinates\"") > ':' > ( positions[assign_coordinates]
                                     | position[assign_coordinates]
                                     ))
     |
     (lit("\"arcs\"") > lit(':') > rings_array[assign_rings])
     |
     (lit("\"properties\"") > ':' > properties[assign_properties])
     |
     (omit[json_string] > lit(':') > omit[json_value])) % lit(',')
    ;

MAPNIK_SPIRIT_LOCAL_RULE(geometry, topojson::geometry)
    = '{' > geometry_tuple[create_geometry] > '}'
    ;

MAPNIK_SPIRIT_LOCAL_RULE(geometry_collection, std::vector<topojson::geometry>)
    = '{'
    // FIXME "type" need not be the first key, but if we allow it later,
    //       we'll have to check (after the closing '}') that the object
    //       had "type" : "GeometryCollection", and fail otherwise
    >> (lit("\"type\"") >> ':' >> "\"GeometryCollection\"")
    >> *(',' > ( (lit("\"geometries\"") > ':'
                  > '[' > -(geometry[push_geometry] % ',') > ']')
               | (omit[json_string] > ':' > omit[json_value])
               ))
    >> '}'
    ;

// A topology must have an "objects" member whose value is an object.
// This object may have any number of members, whose value must be a geometry object.
MAPNIK_SPIRIT_LOCAL_RULE(objects, std::vector<topojson::geometry>)
    = lit("\"objects\"") > ':'
    > '{'
    > -((omit[json_string] > ':' > ( geometry_collection[push_collection]
                                   | geometry[push_geometry]
                                   )) % ',')
    > '}'
    ;

// Each arc must be an array of two or more positions.
MAPNIK_SPIRIT_LOCAL_RULE(arc, topojson::arc)
    = '[' >> (position % ',') >> ']'
    ;

MAPNIK_SPIRIT_LOCAL_RULE(arcs, std::vector<topojson::arc>)
    = lit("\"arcs\"") > ':' > '[' > -(arc % ',') > ']'
    ;

MAPNIK_SPIRIT_EXTERN_RULE_DEF(start_rule)
    = '{'
    > -(( (lit("\"type\"") > ':' > "\"Topology\"")
        | bbox[assign_bbox]
        | transform[assign_transform]
        | objects[assign_objects]
        | arcs[assign_arcs]
        ) % ',')
    > '}'
    ;

}}}

#endif //MAPNIK_TOPOJSON_GRAMMAR_X3_DEF_HPP
