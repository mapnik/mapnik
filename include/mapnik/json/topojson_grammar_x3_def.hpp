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

#ifndef MAPNIK_JSON_TOPOJSON_GRAMMAR_X3_DEF_HPP
#define MAPNIK_JSON_TOPOJSON_GRAMMAR_X3_DEF_HPP

#include <mapnik/json/unicode_string_grammar_x3.hpp>
#include <mapnik/json/generic_json_grammar_x3.hpp>
#include <mapnik/json/topojson_grammar_x3.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/adapted/struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
MAPNIK_DISABLE_WARNING_POP

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

namespace mapnik { namespace json { namespace grammar {

using index_type = topojson::index_type;
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
        if (coords.template is<std::vector<mapnik::topojson::coordinate>>())
        {
            auto const& points = coords.template get<std::vector<mapnik::topojson::coordinate>>();
            mpt. points = points;
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
        if (arcs.template is<std::vector<index_type>>())
        {
            auto const& arcs_ = arcs.template get<std::vector<index_type>>();
            line.rings = arcs_;
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
        if (arcs.template is<std::vector<std::vector<index_type>>>())
        {
            auto const& arcs_ = arcs.template get<std::vector<std::vector<index_type>>>();
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
        if (arcs.template is<std::vector<std::vector<index_type>>>())
        {
            auto const& arcs_ = arcs.template get<std::vector<std::vector<index_type>>>();
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
        if (arcs.template is<std::vector<std::vector<std::vector<index_type>>>>())
        {
            auto const& arcs_ = arcs.template get<std::vector<std::vector<std::vector<index_type>>>>();
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
auto const& json_string = json::grammar::unicode_string;
// json value
auto const& json_value = json::grammar::value;
}

using coordinates_type = util::variant<topojson::coordinate,std::vector<topojson::coordinate>>;
using arcs_type = util::variant<std::vector<index_type>,
                                std::vector<std::vector<index_type>>,
                                std::vector<std::vector<std::vector<index_type>>>>;

struct topojson_geometry_type_ : x3::symbols<int>
{
    topojson_geometry_type_()
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
} topojson_geometry_type;

// rules
x3::rule<class transform_tag, mapnik::topojson::transform> const transform = "Transform";
x3::rule<class bbox_tag, mapnik::topojson::bounding_box> const bbox = "Bounding Box";
x3::rule<class objects_tag, std::vector<mapnik::topojson::geometry>> const objects= "Objects";
x3::rule<class property_tag, mapnik::topojson::property> const property = "Property";
x3::rule<class properties_tag, mapnik::topojson::properties> const properties = "Properties";
x3::rule<class geometry_tag, mapnik::topojson::geometry> const geometry = "Geometry";
x3::rule<class geometry_collection_tag, std::vector<mapnik::topojson::geometry>> const geometry_collection = "Geometry Collection";
x3::rule<class geometry_tuple_tag,
         std::tuple<int,
                    coordinates_type,
                    arcs_type,
                    mapnik::topojson::properties>> const geometry_tuple = "Geometry Tuple";
x3::rule<class coordinate_tag, mapnik::topojson::coordinate> const coordinate = "Coordinate";
x3::rule<class coordinates_tag, coordinates_type> const coordinates = "Coordinates";
x3::rule<class arc_tag, mapnik::topojson::arc> const arc = "Arc";
x3::rule<class arcs_tag, std::vector<mapnik::topojson::arc>> const arcs = "Arcs";
x3::rule<class ring_type, std::vector<index_type>> const ring = "Ring";
x3::rule<class rings_type, std::vector<std::vector<index_type>>> const rings = "Rings";
x3::rule<class rings_array_type, arcs_type> const rings_array = "Rings Array";

// defs
auto const topology_def = lit('{') >
    -(((lit("\"type\"") > lit(':') > lit("\"Topology\""))
       |
       bbox[assign_bbox]
       |
       transform[assign_transform]
       |
       objects[assign_objects]
       |
       arcs[assign_arcs]) % lit(','))
      > lit('}')
      ;


auto const transform_def = lit("\"transform\"") > lit(':') > lit('{')
    > lit("\"scale\"") > lit(':')
    > lit('[')
    > double_ > lit(',')
    > double_ > lit(']') > lit(',')
    > lit("\"translate\"") > lit(':')
    > lit('[') > double_ > lit(',') > double_ > lit(']')
    > lit('}')
    ;

auto const  bbox_def = lit("\"bbox\"") > lit(':')
    > lit('[') > double_ > lit(',') > double_
    > lit(',') > double_ > lit(',') > double_
    > lit(']')
    ;


// A topology must have an “objects” member whose value is an object.
// This object may have any number of members, whose value must be a geometry object.
auto const objects_def = lit("\"objects\"") > lit(':')
    > lit('{')
    > -((omit[json_string] > ':' > ( geometry_collection[push_collection]
                                   | geometry[push_geometry]
                                   )) % ',')
    > lit('}')
    ;

auto const geometry_tuple_def =
    ((lit("\"type\"") > lit(':') > topojson_geometry_type[assign_geometry_type])
     |
     (lit("\"coordinates\"") > lit(':') > coordinates[assign_coordinates])
     |
     (lit("\"arcs\"") > lit(':') > rings_array[assign_rings])
     |
     properties[assign_properties]
     |
     (omit[json_string] > lit(':') > omit[json_value])) % lit(',')
    ;

auto const geometry_def = lit("{") > geometry_tuple[create_geometry] > lit("}");

auto const geometry_collection_def = (lit('{') >> lit("\"type\"") >> lit(':') >> lit("\"GeometryCollection\"") >> -omit[lit(',') >> bbox])
    > lit(',') > lit("\"geometries\"") > lit(':')
    > lit('[')
    > -(geometry[push_geometry] % lit(','))
    > lit(']')
    > lit('}')
    ;


auto const ring_def = lit('[') >> (int_ % lit(',')) >> lit(']')
    ;
auto const rings_def = lit('[') >> (ring % lit(',')) >> lit(']')
    ;
auto const rings_array_def = (lit('[') >> (rings % lit(',')) >> lit(']'))
    |
    rings
    |
    ring
    ;

auto const property_def = json_string[assign_prop_name] > lit(':') > json_value[assign_prop_value]
    ;

auto const properties_def = lit("\"properties\"")
    > lit(':')
    > lit('{') > (property % lit(',')) > lit('}')
    ;

auto const arcs_def = lit("\"arcs\"") >> lit(':') >> lit('[') >> -( arc % lit(',')) >> lit(']') ;

auto const arc_def = lit('[') >> -(coordinate % lit(',')) >> lit(']') ;

auto const coordinate_def = lit('[') >> double_ >> lit(',') >> double_ >> omit[*(lit(',') >> double_)] >> lit(']');

auto const coordinates_def = (lit('[') >> coordinate % lit(',') >> lit(']')) | coordinate;

BOOST_SPIRIT_DEFINE(
    topology,
    transform,
    bbox,
    objects,
    geometry_tuple,
    geometry,
    geometry_collection,
    ring,
    rings,
    rings_array,
    property,
    properties,
    arcs,
    arc,
    coordinate,
    coordinates
    );

}}}

#endif //MAPNIK_TOPOJSON_GRAMMAR_X3_DEF_HPP
