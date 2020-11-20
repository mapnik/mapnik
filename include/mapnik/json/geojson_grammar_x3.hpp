/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_JSON_GEOJSON_GRAMMAR_X3_HPP
#define MAPNIK_JSON_GEOJSON_GRAMMAR_X3_HPP

#include <mapnik/value/types.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/positions_x3.hpp>
#include <mapnik/geometry/geometry_types.hpp>
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
MAPNIK_DISABLE_WARNING_POP

#include <vector>

namespace mapnik { namespace json {

namespace x3 = boost::spirit::x3;
struct geojson_value;
using geojson_array = std::vector<geojson_value>;
using geojson_object_element = std::pair<int, geojson_value>;
using geojson_object = std::vector<geojson_object_element>;
using geojson_value_base = mapnik::util::variant<value_null,
                                              value_bool,
                                              value_integer,
                                              value_double,
                                              std::string,
                                              mapnik::geometry::geometry_types,
                                              positions,
                                              geojson_array,
                                              geojson_object>;
struct geojson_value : geojson_value_base
{
#if __cpp_inheriting_constructors >= 200802

    using geojson_value_base::geojson_value_base;

#else

    geojson_value() = default;

    template <typename T>
    geojson_value(T && val)
        : geojson_value_base(std::forward<T>(val)) {}

#endif
};

namespace grammar {

using geojson_grammar_type = x3::rule<class geojson_tag, geojson_value>;
using geojson_key_value_type = x3::rule<class geojson_key_value_type_tag, geojson_object_element>;

geojson_grammar_type const geojson_value = "GeoJSON Value";
geojson_key_value_type const geojson_key_value = "GeoJSON Key/Value Type";

BOOST_SPIRIT_DECLARE(geojson_grammar_type, geojson_key_value_type);

}}}

#endif // MAPNIK_JSON_GEOJSON_GRAMMAR_X3_HPP
