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

template <typename Iterator>
topojson_grammar<Iterator>::topojson_grammar()
    : topojson_grammar::base_type(topology, "topojson")
{
    qi::lit_type lit;
    qi::double_type double_;
    qi::int_type int_;
    qi::no_skip_type no_skip;
    qi::omit_type omit;
    qi::_val_type _val;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    standard_wide::char_type char_;
    using qi::fail;
    using qi::on_error;
    using phoenix::construct;

    // generic json types
    value = object | array | string_ | number
        ;

    pairs = key_value % lit(',')
        ;

    key_value = (string_ >> lit(':') >> value)
        ;

    object = lit('{') >> *pairs >> lit('}')
        ;

    array = lit('[')
        >> value >> *(lit(',') >> value)
        >> lit(']')
        ;

    number %= strict_double
        | int__
        | lit("true")[_val = true]
        | lit("false")[_val = false]
        | lit("null")
        ;

    unesc_char.add
        ("\\\"", '\"') // quotation mark
        ("\\\\", '\\') // reverse solidus
        ("\\/", '/')   // solidus
        ("\\b", '\b')  // backspace
        ("\\f", '\f')  // formfeed
        ("\\n", '\n')  // newline
        ("\\r", '\r')  // carrige return
        ("\\t", '\t')  // tab
        ;

    string_ %= lit('"') >> no_skip[*(unesc_char | "\\u" >> hex4 | (char_ - lit('"')))] >> lit('"')
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
        >> -((omit[string_]
              >> lit(':')
              >>  (geometry_collection | geometry)) % lit(','))
        >> lit('}')
        ;

    geometry =
        point |
        linestring |
        polygon |
        multi_point |
        multi_linestring |
        multi_polygon |
        omit[object]
        ;

    geometry_collection =  lit('{')
               >> lit("\"type\"") >> lit(':') >> lit("\"GeometryCollection\"") >> lit(',')
               >> lit("\"geometries\"") >> lit(':') >> lit('[') >> -(geometry % lit(','))
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

    id = lit("\"id\"") >> lit(':') >> omit[value]
        ;

    ring = lit('[') >> -(int_ % lit(',')) >> lit(']')
        ;

    properties = lit("\"properties\"")
        >> lit(':')
        >> (( lit('{') >> attributes >> lit('}')) | object)
        ;

    attributes = (string_ >> lit(':') >> attribute_value) % lit(',')
        ;

    attribute_value %= number | string_  ;

    arcs = lit("\"arcs\"") >> lit(':')
                           >> lit('[') >> -( arc % lit(',')) >> lit(']') ;

    arc = lit('[') >> -(coordinate % lit(',')) >> lit(']') ;

    coordinate = lit('[') >> double_ >> lit(',') >> double_ >> lit(']');

    topology.name("topology");
    transform.name("transform");
    objects.name("objects");
    arc.name("arc");
    arcs.name("arcs");
    value.name("value");
    coordinate.name("coordinate");

    point.name("point");
    multi_point.name("multi_point");
    linestring.name("linestring");
    polygon.name("polygon");
    multi_polygon.name("multi_polygon");
    geometry_collection.name("geometry_collection");

    on_error<fail>
        (
            topology
            , std::clog
            << phoenix::val("Error! Expecting ")
            << _4 // what failed?
            << phoenix::val(" here: \"")
            << where_message_(_3, _2, 16) // where? 16 is max chars to output
            << phoenix::val("\"")
            << std::endl
            );

}

}}
