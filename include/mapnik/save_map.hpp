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

#ifndef MAPNIK_SAVE_MAP_HPP
#define MAPNIK_SAVE_MAP_HPP

// mapnik
#include <mapnik/config.hpp>

// stl
#include <string>

namespace mapnik {
class Map;

MAPNIK_DECL void save_map(Map const& map, std::string const& filename, bool explicit_defaults = false);
MAPNIK_DECL std::string save_map_to_string(Map const& map, bool explicit_defaults = false);
} // namespace mapnik

#endif // MAPNIK_SAVE_MAP_HPP
