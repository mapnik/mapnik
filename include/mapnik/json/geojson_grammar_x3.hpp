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

#ifndef MAPNIK_JSON_GEOJSON_GRAMMAR_X3_HPP
#define MAPNIK_JSON_GEOJSON_GRAMMAR_X3_HPP

#include <mapnik/value/types.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/positions_x3.hpp>
#include <mapnik/geometry/geometry_types.hpp>
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#pragma GCC diagnostic pop

#include <vector>

namespace mapnik { namespace json {

namespace x3 = boost::spirit::x3;
struct json_value;
using json_array = std::vector<json_value>;
using json_object_element = std::pair<int, json_value>;
using json_object = std::vector<json_object_element>;
using json_value_base = mapnik::util::variant<value_null,
                                              value_bool,
                                              value_integer,
                                              value_double,
                                              std::string,
                                              mapnik::geometry::geometry_types,
                                              positions,
                                              json_array,
                                              json_object>;
struct json_value : json_value_base
{
#if __cpp_inheriting_constructors >= 200802

    using json_value_base::json_value_base;

#else

    json_value() = default;

    template <typename T>
    json_value(T && val)
        : json_value_base(std::forward<T>(val)) {}

#endif
};

namespace grammar {

using geojson_grammar_type = x3::rule<class geojson_tag, json_value>;
using key_value_type = x3::rule<class key_value_tag, json_object_element>;
BOOST_SPIRIT_DECLARE(geojson_grammar_type);
BOOST_SPIRIT_DECLARE(key_value_type);
}

grammar::geojson_grammar_type const& geojson_grammar();
grammar::key_value_type const& key_value_grammar();

}}

#endif // MAPNIK_JSON_GEOJSON_GRAMMAR_X3_HPP
