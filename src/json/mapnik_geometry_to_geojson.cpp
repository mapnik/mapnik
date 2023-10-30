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

// mapnik
#include <mapnik/util/geometry_to_geojson.hpp>
#include <mapnik/json/geometry_generator_grammar.hpp>

namespace mapnik {
namespace util {

bool to_geojson(std::string& json, mapnik::geometry::geometry<double> const& geom)
{
    using sink_type = std::back_insert_iterator<std::string>;
    static const mapnik::json::geometry_generator_grammar<sink_type, mapnik::geometry::geometry<double>> grammar;
    sink_type sink(json);
    return boost::spirit::karma::generate(sink, grammar, geom);
}

} // namespace util
} // namespace mapnik
