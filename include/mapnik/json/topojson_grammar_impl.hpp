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

#include <mapnik/topojson_grammar.hpp>

//#define BOOST_SPIRIT_USE_PHOENIX_V3 1
//#include <boost/spirit/include/qi.hpp>
//#include <boost/spirit/include/phoenix.hpp>
//
//#include <mapnik/value.hpp>
//#include <mapnik/topology.hpp>
//
//#include <string>

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
    using qi::lit;
    using qi::double_;
    using qi::int_;
    using qi::no_skip;
    using qi::omit;
    using qi::_val;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::fail;
    using qi::on_error;

    using standard_wide::char_;
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

    objects = lit("\"objects\"") >> lit(':') >> lit('{')
                                 >> omit[string_] >> lit(':') >> lit('{')
                                 >> lit("\"type\"") >> lit(':') >> lit("\"GeometryCollection\"") >> lit(',')
                                 >> lit("\"geometries\"") >> lit(':') >> lit('[') >> -(geometry % lit(','))
                                 >> lit(']') >> lit('}') >> lit('}')
        ;

    geometry = point | linestring | polygon | omit[object]
        ;

    point = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"Point\"")
        >> ((lit(',') >> lit("\"coordinates\"") >> lit(':') >> coordinate) ^ (lit(',') >> properties))
        >> lit('}')
        ;

    linestring = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"LineString\"")
        >> ((lit(',') >> lit("\"arcs\"") >> lit(':') >> lit('[') >> int_ >> lit(']')) ^ (lit(',') >> properties))
        >> lit('}')
        ;

    polygon = lit('{')
        >> lit("\"type\"") >> lit(':') >> lit("\"Polygon\"")
        >> ((lit(',') >> lit("\"arcs\"") >> lit(':') >> lit('[') >> -(ring % lit(',')) >> lit(']')) ^ (lit(',') >> properties))
        >> lit('}')
            ;

    ring = lit('[') >> -(int_ % lit(',')) >> lit(']')
        ;

    properties = lit("\"properties\"")
        >> lit(':')
        >> object
        ;

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
    linestring.name("linestring");
    polygon.name("polygon");

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
