/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_JSON_ATTRIBUTE_VALUE_VISITOR_HPP
#define MAPNIK_JSON_ATTRIBUTE_VALUE_VISITOR_HPP

// mapnik
#include <mapnik/value.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/json/stringifier.hpp>

namespace mapnik {
namespace json {

struct attribute_value_visitor
{
  public:
    attribute_value_visitor(mapnik::transcoder const& tr)
        : tr_(tr)
    {}

    mapnik::value operator()(std::string const& val) const { return mapnik::value(tr_.transcode(val.c_str())); }

    mapnik::value operator()(std::vector<mapnik::json::json_value> const& array) const
    {
        std::string str = stringifier()(array);
        return mapnik::value(tr_.transcode(str.c_str()));
    }

    mapnik::value operator()(std::vector<std::pair<std::string, mapnik::json::json_value>> const& object) const
    {
        std::string str = stringifier()(object);
        return mapnik::value(tr_.transcode(str.c_str()));
    }

    template<typename T>
    mapnik::value operator()(T const& val) const
    {
        return mapnik::value(val);
    }

    mapnik::transcoder const& tr_;
};

} // namespace json
} // namespace mapnik

#endif // MAPNIK_JSON_ATTRIBUTE_VALUE_VISITOR_HPP
