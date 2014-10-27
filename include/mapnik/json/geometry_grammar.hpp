/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_GRAMMAR_HPP
#define MAPNIK_GEOMETRY_GRAMMAR_HPP

// mapnik
#include <mapnik/geometry.hpp>  // for geometry_type
#include <mapnik/vertex.hpp>  // for CommandType
#include <mapnik/make_unique.hpp>
#include <mapnik/json/positions_grammar.hpp>
#include <mapnik/json/geometry_util.hpp>
#include <mapnik/geometry_container.hpp>

// spirit::qi
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;
namespace standard_wide =  boost::spirit::standard_wide;
using standard_wide::space_type;

template <typename Iterator, typename ErrorHandler = error_handler<Iterator> >
struct geometry_grammar :
        qi::grammar<Iterator,void(mapnik::geometry_container& )
        , space_type>
{
    geometry_grammar();
    qi::rule<Iterator,void(mapnik::geometry_container& ), space_type> start;
    qi::rule<Iterator, qi::locals<int, mapnik::json::coordinates>, void(mapnik::geometry_container& ), space_type> geometry;
    qi::symbols<char, int> geometry_type_dispatch;
    qi::rule<Iterator,void(mapnik::geometry_container& ),space_type> geometry_collection;
    positions_grammar<Iterator> coordinates;
    boost::phoenix::function<create_geometry_impl> create_geometry;
    // error handler
    boost::phoenix::function<ErrorHandler> const error_handler;
};

}}

#endif // MAPNIK_GEOMETRY_GRAMMAR_HPP
