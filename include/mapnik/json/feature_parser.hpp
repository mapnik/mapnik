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

#ifndef MAPNIK_JSON_FEATURE_PARSER_HPP
#define MAPNIK_JSON_FEATURE_PARSER_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/json/feature_grammar.hpp>

// boost
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

namespace mapnik { namespace json {

inline bool from_geojson(std::string const& json, mapnik::feature_impl & feature)
{
    static const mapnik::transcoder tr("utf8");
    using iterator_type = char const*;
    static const mapnik::json::feature_grammar<iterator_type,mapnik::feature_impl> g(tr);
    using namespace boost::spirit;
    ascii::space_type space;
    iterator_type start = json.c_str();
    iterator_type end = start + json.length();
    return qi::phrase_parse(start, end, (g)(boost::phoenix::ref(feature)), space);
}

}}

#endif // MAPNIK_JSON_FEATURE_PARSER_HPP
