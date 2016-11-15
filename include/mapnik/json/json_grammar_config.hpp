/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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

#ifndef MAPNIK_JSON_GRAMMAR_CONFIG_HPP
#define MAPNIK_JSON_GRAMMAR_CONFIG_HPP

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#pragma GCC diagnostic pop

#include <unordered_map>

namespace mapnik { namespace json {

enum well_known_names
{
    type = 1,
    geometry,
    coordinates,
    properties,
    id
};

constexpr char const* wkn_to_string(well_known_names val)
{
    switch(val)
    {
    case type: return "type";
    case geometry: return "geometry";
    case coordinates: return "coordinates";
    case properties: return "properties";
    case id: return "id";
    default: return "unknown";
    }
}

struct keys_tag;

using keys_map = std::unordered_map<std::string, int>;

inline keys_map get_keys()
{
    keys_map keys =  {
        {"type", well_known_names::type},
        {"geometry", well_known_names::geometry},
        {"coordinates", well_known_names::coordinates},
        {"properties", well_known_names::properties},
        {"id", well_known_names::id}
    };
    return keys;
}

namespace grammar {

namespace x3 = boost::spirit::x3;
using space_type = x3::standard::space_type;
using iterator_type = std::string::const_iterator;
using context_type = x3::with_context<keys_tag,
                                      std::reference_wrapper<keys_map> const,
                                      x3::phrase_parse_context<space_type>::type>::type;
}}}

#endif // MAPNIK_JSON_GRAMMAR_CONFIG_HPP
