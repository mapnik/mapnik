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

#ifndef MAPNIK_JSON_GEOMETRY_PARSER_HPP
#define MAPNIK_JSON_GEOMETRY_PARSER_HPP

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_container.hpp>
#include <mapnik/json/geometry_grammar.hpp>

// boost
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

namespace mapnik { namespace json {

inline bool from_geojson(std::string const& json, geometry_container & paths)
{
    using namespace boost::spirit;
    static const geometry_grammar<std::string::const_iterator> g;
    standard_wide::space_type space;
    std::string::const_iterator start = json.begin();
    std::string::const_iterator end = json.end();
    return qi::phrase_parse(start, end, (g)(boost::phoenix::ref(paths)), space);
}

}}

#endif // MAPNIK_JSON_GEOMETRY_PARSER_HPP
