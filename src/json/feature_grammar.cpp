/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/json/feature_grammar.hpp>

// boost
#include <boost/spirit/include/support_multi_pass.hpp>

namespace mapnik { namespace json {

template <typename Iterator, typename FeatureType>
feature_grammar<Iterator,FeatureType>::feature_grammar(mapnik::transcoder const& tr)
    : feature_grammar::base_type(feature,"feature"),
      put_property_(put_property(tr))
{
    using qi::lit;
    using qi::int_;
    using qi::double_;
#if BOOST_VERSION > 104200
    using qi::no_skip;
#else
    using qi::lexeme;
#endif
    using standard_wide::char_;
    using qi::_val;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_r1;
    using qi::_r2;
    using qi::fail;
    using qi::on_error;
    using qi::_pass;
    using qi::eps;
    using qi::raw;

    using phoenix::new_;
    using phoenix::push_back;
    using phoenix::construct;

    // generic json types
    value =  object | array | string_
        | number
        ;

    pairs = key_value % lit(',')
        ;

    key_value = (string_ >> lit(':') >> value)
        ;

    object = lit('{')
        >> *pairs
        >> lit('}')
        ;
    array = lit('[')
        >> value >> *(lit(',') >> value)
        >> lit(']')
        ;

    number %= strict_double
        | int_
        | lit("true") [_val = true]
        | lit ("false") [_val = false]
        | lit("null")[_val = construct<value_null>()]
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
#if BOOST_VERSION > 104200
    string_ %= lit('"') >> no_skip[*(unesc_char | "\\u" >> hex4 | (char_ - lit('"')))] >> lit('"')
        ;
#else
    string_ %= lit('"') >> lexeme[*(unesc_char | "\\u" >> hex4 | (char_ - lit('"')))] >> lit('"')
        ;
#endif
    // geojson types

    feature_type = lit("\"type\"")
        >> lit(':')
        >> lit("\"Feature\"")
        ;

    feature = lit('{')
        >> (feature_type | (lit("\"geometry\"") > lit(':') > geometry(_r1)) | properties(_r1) | key_value) % lit(',')
        >> lit('}')
        ;

    properties = lit("\"properties\"")
        >> lit(':') >> (lit('{') >>  attributes(_r1) >> lit('}')) | lit("null")
        ;

    attributes = (string_ [_a = _1] >> lit(':') >> attribute_value [put_property_(_r1,_a,_1)]) % lit(',')
        ;

    attribute_value %= number | string_  ;

    // Nabialek trick - FIXME: how to bind argument to dispatch rule?
    // geometry = lit("\"geometry\"")
    //    >> lit(':') >> lit('{')
    //    >> lit("\"type\"") >> lit(':') >> geometry_dispatch[_a = _1]
    //    >> lit(',') >> lit("\"coordinates\"") >> lit(':')
    //    >> qi::lazy(*_a)
    //    >> lit('}')
    //    ;
    // geometry_dispatch.add
    //    ("\"Point\"",&point_coordinates)
    //    ("\"LineString\"",&linestring_coordinates)
    //    ("\"Polygon\"",&polygon_coordinates)
    //    ;
    //////////////////////////////////////////////////////////////////

    geometry = (lit('{')[_a = 0 ]
                >> lit("\"type\"") >> lit(':') >> geometry_dispatch[_a = _1] // <---- should be Nabialek trick!
                >> lit(',')
                >> (lit("\"coordinates\"") > lit(':') > coordinates(_r1,_a)
                    |
                    lit("\"geometries\"") > lit(':')
                    >> lit('[') >> geometry_collection(_r1) >> lit(']'))
                >> lit('}'))
        | lit("null")
        ;

    geometry_dispatch.add
        ("\"Point\"",1)
        ("\"LineString\"",2)
        ("\"Polygon\"",3)
        ("\"MultiPoint\"",4)
        ("\"MultiLineString\"",5)
        ("\"MultiPolygon\"",6)
        ("\"GeometryCollection\"",7)
        //
        ;

    coordinates = (eps(_r2 == 1) > point_coordinates(extract_geometry_(_r1)))
        | (eps(_r2 == 2) > linestring_coordinates(extract_geometry_(_r1)))
        | (eps(_r2 == 3) > polygon_coordinates(extract_geometry_(_r1)))
        | (eps(_r2 == 4) > multipoint_coordinates(extract_geometry_(_r1)))
        | (eps(_r2 == 5) > multilinestring_coordinates(extract_geometry_(_r1)))
        | (eps(_r2 == 6) > multipolygon_coordinates(extract_geometry_(_r1)))
        ;

    point_coordinates =  eps[ _a = new_<geometry_type>(Point) ]
        > ( point(SEG_MOVETO,_a) [push_back(_r1,_a)] | eps[cleanup_(_a)][_pass = false] )
        ;

    linestring_coordinates = eps[ _a = new_<geometry_type>(LineString)]
        > -(points(_a) [push_back(_r1,_a)]
           | eps[cleanup_(_a)][_pass = false])
        ;

    polygon_coordinates = eps[ _a = new_<geometry_type>(Polygon) ]
        > ((lit('[')
            > -(points(_a) % lit(','))
            > lit(']')) [push_back(_r1,_a)]
           | eps[cleanup_(_a)][_pass = false])
        ;

    multipoint_coordinates =  lit('[')
        > -(point_coordinates(_r1) % lit(','))
        > lit(']')
        ;

    multilinestring_coordinates =  lit('[')
        > -(linestring_coordinates(_r1) % lit(','))
        > lit(']')
        ;

    multipolygon_coordinates =  lit('[')
        > -(polygon_coordinates(_r1) % lit(','))
        > lit(']')
        ;

    geometry_collection = *geometry(_r1) >> *(lit(',') >> geometry(_r1))
        ;

    // point
    point = lit('[') > -((double_ > lit(',') > double_)[push_vertex_(_r1,_r2,_1,_2)]) > lit(']');
    // points
    points = lit('[')[_a = SEG_MOVETO] > -(point (_a,_r1) % lit(',')[_a = SEG_LINETO]) > lit(']');
    on_error<fail>
        (
            feature
            , std::clog
            << phoenix::val("Error! Expecting ")
            << _4                               // what failed?
            << phoenix::val(" here: \"")
            << construct<std::string>(_3, _2)   // iterators to error-pos, end
            << phoenix::val("\"")
            << std::endl
            );

}

template struct mapnik::json::feature_grammar<std::string::const_iterator,mapnik::Feature>;
template struct mapnik::json::feature_grammar<boost::spirit::multi_pass<std::istreambuf_iterator<char> >,mapnik::Feature>;

}}
