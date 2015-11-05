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

#ifndef MAPNIK_GEOMETRY_GRAMMAR_HPP
#define MAPNIK_GEOMETRY_GRAMMAR_HPP

// mapnik
#include <mapnik/geometry.hpp>  // for geometry_type
#include <mapnik/make_unique.hpp>
#include <mapnik/json/generic_json.hpp>
#include <mapnik/json/positions_grammar.hpp>
#include <mapnik/json/geometry_util.hpp>

// spirit::qi
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;

template <typename Iterator, typename ErrorHandler = error_handler<Iterator> >
struct geometry_grammar :
        qi::grammar<Iterator, mapnik::geometry::geometry<double>() ,space_type>
{
    geometry_grammar();
    qi::rule<Iterator, mapnik::geometry::geometry<double>(), space_type> start;
    qi::rule<Iterator, qi::locals<int, mapnik::json::coordinates>, mapnik::geometry::geometry<double>(), space_type> geometry;
    qi::rule<Iterator, mapnik::geometry::geometry_collection<double>(), space_type> geometry_collection;
    qi::symbols<char, int> geometry_type_dispatch;
    positions_grammar<Iterator> coordinates;
    boost::phoenix::function<create_geometry_impl> create_geometry;
    // error handler
    ErrorHandler error_handler;
};

}}

#endif // MAPNIK_GEOMETRY_GRAMMAR_HPP
