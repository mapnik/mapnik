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

#ifndef MAPNIK_JSON_JSON_VALUE_HPP
#define MAPNIK_JSON_JSON_VALUE_HPP

// mapnik
#include <mapnik/value/types.hpp>
#include <mapnik/util/variant.hpp>
// stl
#include <string>
#include <vector>

namespace mapnik { namespace json {

struct json_value;

using json_array = std::vector<json_value>;
using json_object_element = std::pair<std::string, json_value>;
using json_object = std::vector<json_object_element>;
using json_value_base = mapnik::util::variant<value_null,
                                              value_bool,
                                              value_integer,
                                              value_double,
                                              std::string,
                                              json_array,
                                              json_object>;
struct json_value : json_value_base
{
#if __cpp_inheriting_constructors >= 200802  && !defined (_MSC_VER)

    using json_value_base::json_value_base;

#else

    json_value() = default;

    template <typename T>
    json_value(T && val)
        : json_value_base(std::forward<T>(val)) {}

#endif
};
}}

#endif // MAPNIK_JSON_JSON_VALUE_HPP
