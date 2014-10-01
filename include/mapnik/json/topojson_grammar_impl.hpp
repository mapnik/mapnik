/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

namespace mapnik { namespace topojson {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;
namespace standard_wide = boost::spirit::standard_wide;
using standard_wide::space_type;

template <typename Iterator, typename ErrorHandler>
topojson_grammar<Iterator, ErrorHandler>::topojson_grammar()
    : topojson_grammar::base_type(topology, "topojson")
{
    qi::lit_type lit;
    qi::double_type double_;
    qi::int_type int_;
    qi::no_skip_type no_skip;
    qi::omit_type omit;
    qi::_val_type _val;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_r1_type _r1;
    standard_wide::char_type char_;
    using qi::fail;
    using qi::on_error;
    using phoenix::push_back;
    using phoenix::construct;
    // generic json types
    json_.value = json_.object | json_.array | json_.string_ | json_.number
        ;

    json_.pairs = json_.key_value % lit(',')
        ;

    json_.key_value = (json_.string_ >> lit(':') >> json_.value)
        ;

    json_.object = lit('{') >> *json_.pairs >> lit('}')
        ;

    json_.array = lit('[')
        >> json_.value >> *(lit(',') >> json_.value)
        >> lit(']')
        ;

    json_.number = json_.strict_double[_val = json_.double_converter(_1)]
        | json_.int__[_val = json_.integer_converter(_1)]
        | lit("true")[_val = true]
        | lit("false")[_val = false]
        | lit("null")[_val = construct<value_null>()]
        ;

    json_.unesc_char.add
        ("\\\"", '\"') // quotation mark
        ("\\\\", '\\') // reverse solidus
        ("\\/", '/')   // solidus
        ("\\b", '\b')  // backspace
        ("\\f", '\f')  // formfeed
        ("\\n", '\n')  // newline
        ("\\r", '\r')  // carrige return
        ("\\t", '\t')  // tab
        ;

    json_.string_ %= lit('"') >> no_skip[*(json_.unesc_char | "\\u" >> json_.hex4 | (char_ - lit('"')))] >> lit('"')
        ;

    // topo json
    topology = lit('{') >> lit("\"type\"") >> lit(':') >> lit("\"Topology\"")
                        >> ( (lit(',') >> objects) ^ ( lit(',') >> arcs) ^ (lit(',') >> transform) ^ (lit(',') >> bbox))
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
        >> -((omit[json_.string_]
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
        omit[json_.object]
        ;

    geometry_collection =  lit('{')
               >> lit("\"type\"") >> lit(':') >> lit("\"GeometryCollection\"") >> lit(',')
               >> lit("\"geometries\"") >> lit(':') >> lit('[') >> -(geometry[push_back(_r1, _1)] % lit(','))
               >> lit(']')
               >> lit('}')
        ;
    point = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"Point\"")
        >> ((lit(',') >> lit("\"coordinates\"") >> lit(':') >> coordinate)
            ^ (lit(',') >> properties) /*^ (lit(',') >> omit[id])*/)
        >> lit('}')
        ;

    multi_point = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"MultiPoint\"")
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
        >> ((lit(',') >> lit("\"arcs\"") >> lit(':') >> lit('[')
             >> -((lit('[') >> int_ >> lit(']')) % lit(',')) >> lit(']'))
            ^ (lit(',') >> properties) ^ (lit(',') >> omit[id]))
        >> lit('}')
        ;

    polygon = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"Polygon\"")
        >> ((lit(',') >> lit("\"arcs\"") >> lit(':')
             >> lit('[') >> -(ring % lit(',')) >> lit(']'))
            ^ (lit(',') >> properties) ^ (lit(',') >> omit[id]))
        >> lit('}')
        ;

    multi_polygon = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"MultiPolygon\"")
        >> ((lit(',') >> lit("\"arcs\"") >> lit(':')
             >> lit('[')
             >> -((lit('[') >> -(ring % lit(',')) >> lit(']')) % lit(','))
             >> lit(']')) ^ (lit(',') >> properties) ^ (lit(',') >> omit[id]))
        >> lit('}')
        ;

    id = lit("\"id\"") >> lit(':') >> omit[json_.value]
        ;

    ring = lit('[') >> -(int_ % lit(',')) >> lit(']')
        ;

    properties = lit("\"properties\"")
        >> lit(':')
        >> (( lit('{') >> attributes >> lit('}')) | json_.object)
        ;

    attributes = (json_.string_ >> lit(':') >> attribute_value) % lit(',')
        ;

    attribute_value %= json_.number | json_.string_  ;

    arcs = lit("\"arcs\"") >> lit(':')
                           >> lit('[') >> -( arc % lit(',')) >> lit(']') ;

    arc = lit('[') >> -(coordinate % lit(',')) >> lit(']') ;

    coordinate = lit('[') >> double_ >> lit(',') >> double_ >> lit(']');

    topology.name("topology");
    transform.name("transform");
    objects.name("objects");
    arc.name("arc");
    arcs.name("arcs");
    json_.value.name("value");
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
