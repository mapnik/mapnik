/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_WKT_GRAMMAR_HPP
#define MAPNIK_WKT_GRAMMAR_HPP

#include <boost/assert.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
// spirit::qi
#include <boost/spirit/include/qi.hpp>
// spirit::phoenix
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
// mapnik
#include <mapnik/geometry.hpp>

namespace mapnik { namespace wkt {

    using namespace boost::spirit;
    using namespace boost::phoenix;

    struct push_vertex
    {
#ifdef BOOST_SPIRIT_USE_PHOENIX_V3
        template <typename T>
#else
        template <typename T0,typename T1, typename T2, typename T3>
#endif
        struct result
        {
            typedef void type;
        };

        template <typename T0,typename T1, typename T2, typename T3>
        void operator() (T0 c, T1 path, T2 x, T3 y) const
        {
            BOOST_ASSERT( path!=0 );
            path->push_vertex(x,y,c);
        }
    };

    struct close_path
    {
        template <typename T>
        struct result
        {
            typedef void type;
        };

        template <typename T>
        void operator() (T path) const
        {
            BOOST_ASSERT( path!=0 );
            path->close_path();
        }
    };

    struct cleanup
    {
        template <typename T0>
        struct result
        {
            typedef void type;
        };

        template <typename T0>
        void operator() (T0 & path) const
        {
            if (path)  delete path,path=0;
        }
    };

    template <typename Iterator>
    struct wkt_grammar : qi::grammar<Iterator,  boost::ptr_vector<mapnik::geometry_type>() , ascii::space_type>
    {
        wkt_grammar()
            : wkt_grammar::base_type(geometry_tagged_text)
        {
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_pass_type _pass;
            qi::eps_type eps;
            qi::_val_type _val;
            qi::lit_type lit;
            qi::no_case_type no_case;
            qi::double_type double_;
            qi::_1_type _1;
            qi::_2_type _2;
            qi::_a_type _a;
            using boost::phoenix::push_back;

            geometry_tagged_text = point_tagged_text
                | linestring_tagged_text
                | polygon_tagged_text
                | multipoint_tagged_text
                | multilinestring_tagged_text
                | multipolygon_tagged_text
                ;

            // <point tagged text> ::= point <point text>
            point_tagged_text = no_case[lit("POINT")] [ _a = new_<geometry_type>(geometry_type::types::Point) ]
                >> ( point_text(_a) [push_back(_val,_a)]
                     | eps[cleanup_(_a)][_pass = false])
                ;

            // <point text> ::= <empty set> | <left paren> <point> <right paren>
            point_text = (lit("(") >> point(SEG_MOVETO,_r1) >> lit(')'))
                | empty_set
                ;

            // <linestring tagged text> ::= linestring <linestring text>
            linestring_tagged_text = no_case[lit("LINESTRING")] [ _a = new_<geometry_type>(geometry_type::types::LineString) ]
                >> (linestring_text(_a)[push_back(_val,_a)]
                    | eps[cleanup_(_a)][_pass = false])
                ;

            // <linestring text> ::= <empty set> | <left paren> <point> {<comma> <point>}* <right paren>
            linestring_text = points(_r1) | empty_set
                ;

            // <polygon tagged text> ::= polygon <polygon text>
            polygon_tagged_text = no_case[lit("POLYGON")] [ _a = new_<geometry_type>(geometry_type::types::Polygon) ]
                >> ( polygon_text(_a)[push_back(_val,_a)]
                     | eps[cleanup_(_a)][_pass = false])
                ;

            // <polygon text> ::= <empty set> | <left paren> <linestring text> {<comma> <linestring text>}* <right paren>
            polygon_text = (lit('(') >> linestring_text(_r1)[close_path_(_r1)] % lit(',') >> lit(')')) | empty_set;


            //<multipoint tagged text> ::= multipoint <multipoint text>
            multipoint_tagged_text = no_case[lit("MULTIPOINT")]
                >>  multipoint_text
                ;

            // <multipoint text> ::= <empty set> | <left paren> <point text> {<comma> <point text>}* <right paren>
            multipoint_text = (lit('(')
                               >> ((eps[_a = new_<geometry_type>(geometry_type::types::Point)]
                                    >> (point_text(_a) | empty_set) [push_back(_val,_a)]
                                    | eps[cleanup_(_a)][_pass = false]) % lit(','))
                               >> lit(')')) | empty_set
                ;

            // <multilinestring tagged text> ::= multilinestring <multilinestring text>
            multilinestring_tagged_text = no_case[lit("MULTILINESTRING")]
                >> multilinestring_text ;

            // <multilinestring text> ::= <empty set> | <left paren> <linestring text> {<comma> <linestring text>}* <right paren>
            multilinestring_text = (lit('(')
                                    >> ((eps[_a = new_<geometry_type>(geometry_type::types::LineString)]
                                         >> ( points(_a)[push_back(_val,_a)]
                                              | eps[cleanup_(_a)][_pass = false]))
                                        % lit(','))
                                    >> lit(')')) | empty_set;

            // <multipolygon tagged text> ::= multipolygon <multipolygon text>
            multipolygon_tagged_text = no_case[lit("MULTIPOLYGON")]
                >> multipolygon_text ;

            // <multipolygon text> ::= <empty set> | <left paren> <polygon text> {<comma> <polygon text>}* <right paren>

            multipolygon_text = (lit('(')
                                 >> ((eps[_a = new_<geometry_type>(geometry_type::types::Polygon)]
                                      >> ( polygon_text(_a)[push_back(_val,_a)]
                                           | eps[cleanup_(_a)][_pass = false]))
                                     % lit(','))
                                 >> lit(')')) | empty_set;

            // points
            points = lit('(')[_a = SEG_MOVETO] >> point (_a,_r1) % lit(',') [_a = SEG_LINETO]  >> lit(')');
            // point
            point = (double_ >> double_) [push_vertex_(_r1,_r2,_1,_2)];

            // <empty set>
            empty_set = no_case[lit("EMPTY")];

        }

        // start
        qi::rule<Iterator,boost::ptr_vector<geometry_type>(),ascii::space_type> geometry_tagged_text;

        qi::rule<Iterator,qi::locals<geometry_type*>,boost::ptr_vector<geometry_type>(),ascii::space_type> point_tagged_text;
        qi::rule<Iterator,qi::locals<geometry_type*>,boost::ptr_vector<geometry_type>(),ascii::space_type> linestring_tagged_text;
        qi::rule<Iterator,qi::locals<geometry_type*>,boost::ptr_vector<geometry_type>(),ascii::space_type> polygon_tagged_text;
        qi::rule<Iterator,boost::ptr_vector<geometry_type>(),ascii::space_type> multipoint_tagged_text;
        qi::rule<Iterator,boost::ptr_vector<geometry_type>(),ascii::space_type> multilinestring_tagged_text;
        qi::rule<Iterator,boost::ptr_vector<geometry_type>(),ascii::space_type> multipolygon_tagged_text;
        //
        qi::rule<Iterator,void(geometry_type*),ascii::space_type> point_text;
        qi::rule<Iterator,void(geometry_type*),ascii::space_type> linestring_text;
        qi::rule<Iterator,void(geometry_type*),ascii::space_type> polygon_text;
        qi::rule<Iterator, qi::locals<geometry_type*>, boost::ptr_vector<geometry_type>(),ascii::space_type> multipoint_text;
        qi::rule<Iterator, qi::locals<geometry_type*>, boost::ptr_vector<geometry_type>(),ascii::space_type> multilinestring_text;
        qi::rule<Iterator, qi::locals<geometry_type*>, boost::ptr_vector<geometry_type>(),ascii::space_type> multipolygon_text;
        //
        qi::rule<Iterator,void(CommandType,geometry_type*),ascii::space_type> point;
        qi::rule<Iterator,qi::locals<CommandType>,void(geometry_type*),ascii::space_type> points;
        qi::rule<Iterator,ascii::space_type> empty_set;
        boost::phoenix::function<push_vertex> push_vertex_;
        boost::phoenix::function<close_path> close_path_;
        boost::phoenix::function<cleanup> cleanup_;
    };


template <typename Iterator>
struct wkt_collection_grammar : qi::grammar<Iterator, boost::ptr_vector<mapnik::geometry_type>(), ascii::space_type>
{
    wkt_collection_grammar()
        :  wkt_collection_grammar::base_type(start)
    {
        qi::lit_type lit;
        qi::no_case_type no_case;
        using boost::phoenix::push_back;
        start = wkt | no_case[lit("GEOMETRYCOLLECTION")]
            >> (lit("(") >> wkt % lit(",") >> lit(")"));
    }

    qi::rule<Iterator,boost::ptr_vector<mapnik::geometry_type>(),ascii::space_type> start;
    wkt_grammar<Iterator> wkt;
};

template <typename Iterator>
struct wkt_stream_grammar : qi::grammar<Iterator, boost::ptr_vector<mapnik::geometry_type>(), ascii::space_type>
{
    wkt_stream_grammar()
        :  wkt_stream_grammar::base_type(start)
    {
        start = *wkt_collection;
    }

    qi::rule<Iterator,boost::ptr_vector<mapnik::geometry_type>(),ascii::space_type> start;
    wkt_collection_grammar<Iterator> wkt_collection;
};

}}

#endif // MAPNIK_WKT_GRAMMAR_HPP
