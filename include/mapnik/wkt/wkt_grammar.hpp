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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/assert.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_container.hpp>
#include <mapnik/vertex.hpp>

namespace mapnik { namespace wkt {

using namespace boost::spirit;

struct push_vertex
{
    template <typename T>
    struct result
    {
        using type = void;
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
        using type = void;
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
        using type = void;
    };

    template <typename T0>
    void operator() (T0 & path) const
    {
        if (path)  delete path,path=0;
    }
};

template <typename Iterator>
struct wkt_grammar : qi::grammar<Iterator,  mapnik::geometry_container() , ascii::space_type>
{
    wkt_grammar();
    qi::rule<Iterator,mapnik::geometry_container(),ascii::space_type> geometry_tagged_text;
    qi::rule<Iterator,qi::locals<geometry_type*>,mapnik::geometry_container(),ascii::space_type> point_tagged_text;
    qi::rule<Iterator,qi::locals<geometry_type*>,mapnik::geometry_container(),ascii::space_type> linestring_tagged_text;
    qi::rule<Iterator,qi::locals<geometry_type*>,mapnik::geometry_container(),ascii::space_type> polygon_tagged_text;
    qi::rule<Iterator,mapnik::geometry_container(),ascii::space_type> multipoint_tagged_text;
    qi::rule<Iterator,mapnik::geometry_container(),ascii::space_type> multilinestring_tagged_text;
    qi::rule<Iterator,mapnik::geometry_container(),ascii::space_type> multipolygon_tagged_text;
    qi::rule<Iterator,void(geometry_type*),ascii::space_type> point_text;
    qi::rule<Iterator,void(geometry_type*),ascii::space_type> linestring_text;
    qi::rule<Iterator,void(geometry_type*),ascii::space_type> polygon_text;
    qi::rule<Iterator, qi::locals<geometry_type*>, mapnik::geometry_container(),ascii::space_type> multipoint_text;
    qi::rule<Iterator, qi::locals<geometry_type*>, mapnik::geometry_container(),ascii::space_type> multilinestring_text;
    qi::rule<Iterator, qi::locals<geometry_type*>, mapnik::geometry_container(),ascii::space_type> multipolygon_text;
    qi::rule<Iterator,void(CommandType,geometry_type*),ascii::space_type> point;
    qi::rule<Iterator,qi::locals<CommandType>,void(geometry_type*),ascii::space_type> points;
    qi::rule<Iterator,ascii::space_type> empty_set;
    boost::phoenix::function<push_vertex> push_vertex_;
    boost::phoenix::function<close_path> close_path_;
    boost::phoenix::function<cleanup> cleanup_;
};

template <typename Iterator>
struct wkt_collection_grammar : qi::grammar<Iterator, mapnik::geometry_container(), ascii::space_type>
{
    wkt_collection_grammar();
    qi::rule<Iterator,mapnik::geometry_container(),ascii::space_type> start;
    wkt_grammar<Iterator> wkt;
};

}}

#endif // MAPNIK_WKT_GRAMMAR_HPP
