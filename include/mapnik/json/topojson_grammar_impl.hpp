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

#include <mapnik/json/topojson_grammar.hpp>
#include <mapnik/json/generic_json.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#pragma GCC diagnostic pop

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

namespace mapnik { namespace topojson {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;

template <typename Iterator, typename ErrorHandler>
topojson_grammar<Iterator, ErrorHandler>::topojson_grammar()
    : topojson_grammar::base_type(topology, "topojson")
{
    qi::lit_type lit;
    qi::double_type double_;
    qi::int_type int_;
    qi::omit_type omit;
    qi::_val_type _val;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_r1_type _r1;
    using qi::fail;
    using qi::on_error;
    using phoenix::push_back;
    using phoenix::construct;

    // error handler
    boost::phoenix::function<ErrorHandler> const error_handler;

    // generic JSON types
    json.value = json.object | json.array | json.string_ | json.number
        ;

    json.pairs = json.key_value % lit(',')
        ;

    json.key_value = (json.string_ >> lit(':') >> json.value)
        ;

    json.object = lit('{') >> *json.pairs >> lit('}')
        ;

    json.array = lit('[')
        >> json.value >> *(lit(',') >> json.value)
        >> lit(']')
        ;

    json.number = json.strict_double[_val = json.double_converter(_1)]
        | json.int__[_val = json.integer_converter(_1)]
        | lit("true")[_val = true]
        | lit("false")[_val = false]
        | lit("null")[_val = construct<value_null>()]
        ;

    // topo json
    topology = lit('{') >> lit("\"type\"") >> lit(':') >> lit("\"Topology\"")
                        >> ( (lit(',') >> objects ) ^ ( lit(',') >> arcs) ^ (lit(',') >> transform) ^ (lit(',') >> bbox))
                        >> lit('}')
        ;

    transform = lit("\"transform\"") >> lit(':') >> lit('{')
                                     >> lit("\"scale\"") >> lit(':')
                                     >> lit('[')
                                     >> double_ >> lit(',')
                                     >> double_ >> lit(']') >> lit(',')
                                     >> lit("\"translate\"") >> lit(':')
                                     >> lit('[') >> double_ >> lit(',') >> double_ >> lit(']')
                                     >> lit('}')
        ;

    bbox = lit("\"bbox\"") >> lit(':')
                           >> lit('[') >> double_ >> lit(',') >> double_
                           >> lit(',') >> double_ >> lit(',') >> double_
                           >> lit(']')
        ;

    objects = lit("\"objects\"")
        >> lit(':')
        >> lit('{')
        >> -((omit[json.string_]
              >> lit(':')
              >>  (geometry_collection(_val) | geometry)) % lit(','))
        >> lit('}')
        ;

    geometry =
        point |
        linestring |
        polygon |
        multi_point |
        multi_linestring |
        multi_polygon |
        omit[json.object]
        ;

    geometry_collection =  lit('{')
               >> lit("\"type\"") >> lit(':') >> lit("\"GeometryCollection\"")
               >> -(lit(',') >> omit[bbox])
               >> lit(',') >> lit("\"geometries\"") >> lit(':') >> lit('[') >> -(geometry[push_back(_r1, _1)] % lit(','))
               >> lit(']')
               >> lit('}')
        ;
    point = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"Point\"")
        >> -(lit(',') >> omit[bbox])
        >> ((lit(',') >> lit("\"coordinates\"") >> lit(':') >> coordinate)
            ^ (lit(',') >> properties) /*^ (lit(',') >> omit[id])*/)
        >> lit('}')
        ;

    multi_point = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"MultiPoint\"")
        >> -(lit(',') >> omit[bbox])
        >> ((lit(',') >> lit("\"coordinates\"") >> lit(':')
             >> lit('[') >> -(coordinate % lit(',')) >> lit(']'))
            ^ (lit(',') >> properties) ^ (lit(',') >> omit[id]))
        >> lit('}')
        ;

    linestring = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"LineString\"")
        >> ((lit(',') >> lit("\"arcs\"") >> lit(':') >> lit('[') >> int_ >> lit(']'))
            ^ (lit(',') >> properties) ^ (lit(',') >> omit[id]))
        >> lit('}')
        ;

    multi_linestring = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"MultiLineString\"")
        >> -(lit(',') >> omit[bbox])
        >> ((lit(',') >> lit("\"arcs\"") >> lit(':') >> lit('[')
             >> -((lit('[') >> int_ >> lit(']')) % lit(',')) >> lit(']'))
            ^ (lit(',') >> properties) ^ (lit(',') >> omit[id]))
        >> lit('}')
        ;

    polygon = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"Polygon\"")
        >> -(lit(',') >> omit[bbox])
        >> ((lit(',') >> lit("\"arcs\"") >> lit(':')
             >> lit('[') >> -(ring % lit(',')) >> lit(']'))
            ^ (lit(',') >> properties) ^ (lit(',') >> omit[id]))
        >> lit('}')
        ;

    multi_polygon = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"MultiPolygon\"")
        >> -(lit(',') >> omit[bbox])
        >> ((lit(',') >> lit("\"arcs\"") >> lit(':')
             >> lit('[')
             >> -((lit('[') >> -(ring % lit(',')) >> lit(']')) % lit(','))
             >> lit(']')) ^ (lit(',') >> properties) ^ (lit(',') >> omit[id]))
        >> lit('}')
        ;

    id = lit("\"id\"") >> lit(':') >> omit[json.value]
        ;

    ring = lit('[') >> -(int_ % lit(',')) >> lit(']')
        ;

    properties = lit("\"properties\"")
        >> lit(':')
        >> (( lit('{') >> attributes >> lit('}')) | json.object)
        ;

    attributes = (json.string_ >> lit(':') >> attribute_value) % lit(',')
        ;

    attribute_value %= json.number | json.string_  ;

    arcs = lit("\"arcs\"") >> lit(':')
                           >> lit('[') >> -( arc % lit(',')) >> lit(']') ;

    arc = lit('[') >> -(coordinate % lit(',')) >> lit(']') ;

    coordinate = lit('[') >> double_ >> lit(',') >> double_ >> lit(']');

    topology.name("topology");
    transform.name("transform");
    objects.name("objects");
    arc.name("arc");
    arcs.name("arcs");
    json.value.name("value");
    coordinate.name("coordinate");

    point.name("point");
    multi_point.name("multi_point");
    linestring.name("linestring");
    polygon.name("polygon");
    multi_polygon.name("multi_polygon");
    geometry_collection.name("geometry_collection");
    // error handler
    on_error<fail>(topology, error_handler(_1, _2, _3, _4));
}

}}
