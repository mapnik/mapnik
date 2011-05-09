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
#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
// mapnik
#include <mapnik/geometry.hpp>

namespace mapnik { namespace wkt {

using namespace boost::spirit;
using namespace boost::fusion;
using namespace boost::phoenix;

struct push_vertex
{
    template <typename T0,typename T1, typename T2, typename T3>
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
        if (path) delete path,path=0;
    }
};

template <typename Iterator>
struct wkt_grammar : qi::grammar<Iterator, geometry_type*(), ascii::space_type>
{
    wkt_grammar()
        : wkt_grammar::base_type(start)
    {       
        using qi::_1;   
        using qi::_2;
        using qi::_val;
        using qi::no_case;   
        
        start %= point | linestring | polygon | multipoint | multilinestring | multipolygon | eps[cleanup_(_val)][_pass = false];
        
        // <point tagged text> ::= point <point text>
        point = no_case[lit("POINT")] [ _val = new_<geometry_type>(Point) ]
            >> (empty_set | lit('(') >> coord(SEG_MOVETO,_val) >> lit(')'));
        
        // <linestring tagged text> ::= linestring <linestring text>
        linestring = no_case[lit("LINESTRING")]  [ _val = new_<geometry_type>(LineString) ] >> (empty_set | points(_val));
        
        // <polygon tagged text> ::= polygon <polygon text>
        polygon = no_case[lit("POLYGON")] [ _val = new_<geometry_type>(Polygon) ] >> 
            (empty_set | lit('(') >> points(_val) % lit(',') >> lit(')'));
        
        // multi point
        multipoint = no_case[lit("MULTIPOINT")]  [ _val = new_<geometry_type>(MultiPoint) ]
            >> (empty_set | lit('(') >> coord(SEG_MOVETO,_val) % lit(',') >> ')');
        
        // multi linestring
        multilinestring = no_case[lit("MULTILINESTRING")] [ _val = new_<geometry_type>(MultiLineString) ]>> 
            (empty_set | lit('(') >> points(_val) % lit(',') >> ')');
        
        // multi polygon
        multipolygon = no_case[lit("MULTIPOLYGON")] [ _val = new_<geometry_type>(MultiPolygon)] >> 
            (empty_set | lit('(') >> (lit('(') >> 
                                      points(_val) % ',' >> ')') % ',' >> ')');
        
        // points
        points = lit('(')[_a = SEG_MOVETO] >> coord (_a,_r1) % lit(',') [_a = SEG_LINETO]  >> ')';
        coord = (double_ >> double_) [push_vertex_(_r1,_r2,_1,_2)];
        
        // <empty set>
        empty_set = no_case[lit("EMPTY")];

    }
    
    qi::rule<Iterator,geometry_type*(),ascii::space_type> start;
    qi::rule<Iterator,geometry_type*(),ascii::space_type> point;
    qi::rule<Iterator,geometry_type*(),ascii::space_type> multipoint;
    qi::rule<Iterator,geometry_type*(),ascii::space_type> linestring;
    qi::rule<Iterator,geometry_type*(),ascii::space_type> multilinestring;
    qi::rule<Iterator,geometry_type*(),ascii::space_type> polygon;
    qi::rule<Iterator,geometry_type*(),ascii::space_type> multipolygon;
    qi::rule<Iterator,void(CommandType,geometry_type*),ascii::space_type> coord;
    qi::rule<Iterator,qi::locals<CommandType>,void(geometry_type*),ascii::space_type> points;
    qi::rule<Iterator,ascii::space_type> empty_set;
    boost::phoenix::function<push_vertex> push_vertex_;
    boost::phoenix::function<cleanup> cleanup_;
};

template <typename Iterator>
struct wkt_collection_grammar : qi::grammar<Iterator, boost::ptr_vector<geometry_type>(), ascii::space_type>
{
    wkt_collection_grammar()
        :  wkt_collection_grammar::base_type(start)
    {
        using qi::_1;   
        using qi::_val;
        using qi::no_case; 
        using boost::phoenix::push_back;
        start = wkt [push_back(_val,_1)] | no_case[lit("GEOMETRYCOLLECTION")] 
            >> (lit("(") >> *wkt[push_back(_val,_1)] % lit(",") >> lit(")"));
    }
    
    qi::rule<Iterator,boost::ptr_vector<geometry_type>(),ascii::space_type> start;
    wkt_grammar<Iterator> wkt;
};

}}

#endif // MAPNIK_WKT_GRAMMAR_HPP
