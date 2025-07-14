/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/feature_grammar_x3.hpp>

namespace mapnik {
namespace json {

bool from_geojson(std::string const& json, mapnik::geometry::geometry<double>& geom)
{
    namespace x3 = boost::spirit::x3;
    using space_type = mapnik::json::grammar::space_type;
    auto grammar = mapnik::json::grammar::geometry_rule;
    try
    {
        char const* start = json.c_str();
        char const* end = start + json.length();
        if (!x3::phrase_parse(start, end, grammar, space_type(), geom))
        {
            throw std::runtime_error("Can't parser GeoJSON Geometry");
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}

} // namespace json
} // namespace mapnik
