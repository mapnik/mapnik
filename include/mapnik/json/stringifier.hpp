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

#ifndef MAPNIK_JSON_STRINGIFIER_HPP
#define MAPNIK_JSON_STRINGIFIER_HPP

// mapnik
#include <mapnik/json/generic_json.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/variant.hpp>
// stl
#include <string>


namespace mapnik { namespace json {

struct stringifier
{
    std::string operator()(std::string const& val) const
    {
        return "\"" + val + "\"";
    }

    std::string operator()(value_null) const
    {
        return "null";
    }

    std::string operator()(value_bool val) const
    {
        return val ? "true" : "false";
    }

    std::string operator()(value_integer val) const
    {
        std::string str;
        util::to_string(str, val);
        return str;
    }

    std::string operator()(value_double val) const
    {
        std::string str;
        util::to_string(str, val);
        return str;
    }

    std::string operator()(std::vector<mapnik::json::json_value> const& array) const
    {
        std::string str = "[";
        bool first = true;
        for (auto const& val : array)
        {
            if (first) first = false;
            else str += ",";
            str += mapnik::util::apply_visitor(*this, val);
        }
        str += "]";
        return str;
    }

    std::string operator()(std::vector<std::pair<std::string, mapnik::json::json_value>> const& object) const
    {
        std::string str = "{";
        bool first = true;
        for (auto const& kv : object)
        {
            if (first) first = false;
            else str += ",";
            str +=  "\"" + kv.first + "\"";
            str += ":";
            str += mapnik::util::apply_visitor(*this, kv.second);
        }
        str += "}";
        return str;
    }
};

}}


#endif // MAPNIK_JSON_STRINGIFIER_HPP
