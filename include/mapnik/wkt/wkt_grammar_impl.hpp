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

#include <mapnik/wkt/wkt_grammar.hpp>
#include <mapnik/geometry_fusion_adapted.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/phoenix/object/construct.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace wkt {

using namespace boost::spirit;

template <typename Iterator>
wkt_grammar<Iterator>::wkt_grammar()
    : wkt_grammar::base_type(geometry_tagged_text)
{
    qi::eps_type eps;
    qi::_r1_type _r1;
    qi::_val_type _val;
    qi::lit_type lit;
    qi::no_case_type no_case;
    qi::double_type double_;
    qi::_1_type _1;
    qi::_a_type _a;
    using boost::phoenix::construct;
    using boost::phoenix::at_c;
    geometry_tagged_text = point_tagged_text(_r1)
        | linestring_tagged_text(_r1)
        | polygon_tagged_text(_r1)
        | multipoint_tagged_text(_r1)
        | multilinestring_tagged_text(_r1)
        | multipolygon_tagged_text(_r1)
        | geometrycollection_tagged_text(_r1)
        ;

    // <point tagged text> ::= point <point text>
    point_tagged_text = no_case[lit("POINT")]
        >> (point_text[assign(_r1,_1)] | empty_set[assign(_r1,construct<geometry::geometry_empty>())])
        ;
    // <point text> ::= <empty set> | <left paren> <point> <right paren>
    point_text = (lit("(") >> point >> lit(')'))
        //| empty_set - we're catching 'POINT EMPTY' case in point_tagged_text rule ^^ by creating geometry_empty
        // because our geometry::point<double> can't be empty
        ;

    //<linestring tagged text> ::= linestring <linestring text>
    linestring_tagged_text = no_case[lit("LINESTRING")]
        >> linestring_text[assign(_r1,_1)]
        ;

    // <linestring text> ::= <empty set> | <left paren> <point> {<comma> <point>}* <right paren>
    linestring_text = points | empty_set
        ;

    // <polygon tagged text> ::= polygon <polygon text>
    polygon_tagged_text = no_case[lit("POLYGON")]
        >> polygon_text[assign(_r1,_1)]
        ;

    // <polygon text> ::= <empty set> | <left paren> <linestring text> {<comma> <linestring text>}* <right paren>
    polygon_text =
        (lit('(') >> linearring_text[set_exterior(_val,_1)] >> *(lit(',') >> linearring_text[add_hole(_val,_1)]) >> lit(')'))
        |
        empty_set
        ;

    linearring_text = ring_points | empty_set
        ;
    //<multipoint tagged text> ::= multipoint <multipoint text>
    multipoint_tagged_text = no_case[lit("MULTIPOINT")]
        >>  multipoint_text[assign(_r1,_1)]
        ;

    // <multipoint text> ::= <empty set> | <left paren> <point text> {<comma> <point text>}* <right paren>
    multipoint_text = (lit('(')
                       >> point_text % lit(',')
                       >> lit(')'))
        |
        (lit('(')
         >> point % lit(',')
         >> lit(')'))
        |
        empty_set
        ;

    // <multilinestring tagged text> ::= multilinestring <multilinestring text>
    multilinestring_tagged_text = no_case[lit("MULTILINESTRING")]
        >> multilinestring_text[assign(_r1,_1)] ;

    // <multilinestring text> ::= <empty set> | <left paren> <linestring text> {<comma> <linestring text>}* <right paren>
    multilinestring_text = (lit('(')
                            >> points[move_part(_val,_1)] % lit(',')
                            >> lit(')')) | empty_set;

    // <multipolygon tagged text> ::= multipolygon <multipolygon text>
    multipolygon_tagged_text = no_case[lit("MULTIPOLYGON")]
        >> multipolygon_text[assign(_r1,_1)] ;

    //<multipolygon text> ::= <empty set> | <left paren> <polygon text> {<comma> <polygon text>}* <right paren>
    multipolygon_text = (lit('(')
                         >> polygon_text[move_part(_val,_1)] % lit(',')
                         >> lit(')'))
        |
        empty_set;

    // geometry collection tagged text
    geometrycollection_tagged_text = no_case[lit("GEOMETRYCOLLECTION")]
        >> geometrycollection_text[assign(_r1,_1)]
        ;

    // geometry collection text
    geometrycollection_text = (lit('(')
                               >> ( eps[_a = construct<geometry::geometry<double> >()]
                                    >> geometry_tagged_text(_a)[move_part(_val,_a)] % lit(','))
                               >> lit(')'))
        |
        empty_set;
    // points
    points = lit('(') >> point % lit(',') >> lit(')')
        ;
    // ring points
    ring_points = lit('(') >> point % lit(',') >> lit(')')
        ;
    // point
    point = double_ >> double_
        ;

    // <empty set>
    empty_set = no_case[lit("EMPTY")];
}

}}
