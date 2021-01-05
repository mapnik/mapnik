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

#ifndef MAPNIK_JSON_POSITIONS_HPP
#define MAPNIK_JSON_POSITIONS_HPP

// mapnik
#include <mapnik/util/variant.hpp>
#include <mapnik/geometry.hpp>

namespace mapnik { namespace json {

struct empty {};

using position = mapnik::geometry::point<double>;
using positions = std::vector<position>;
using coordinates = util::variant<empty, position, positions, std::vector<positions>, std::vector<std::vector<positions> > > ;

}}

#endif // MAPNIK_JSON_POSITIONS_HPP
