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

#ifndef MAPNIK_PARSE_PATH_HPP
#define MAPNIK_PARSE_PATH_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/path_expression.hpp>

// stl
#include <string>
#include <set>

namespace mapnik {

// fwd declare to reduce compile time
class feature_impl;

MAPNIK_DECL path_expression_ptr parse_path(std::string const& str);

struct MAPNIK_DECL path_processor
{
    static std::string evaluate(path_expression const& path, feature_impl const& f);
    static std::string to_string(path_expression const& path);
    static void collect_attributes(path_expression const& path, std::set<std::string>& names);
};

using path_processor_type = mapnik::path_processor;

} // namespace mapnik

#endif // MAPNIK_PARSE_PATH_HPP
