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

#ifndef MAPNIK_PROJ_TRANSFORM_CACHE_HPP
#define MAPNIK_PROJ_TRANSFORM_CACHE_HPP

#include <mapnik/config.hpp>
#include <string>

namespace mapnik {
class proj_transform; // fwd decl
namespace proj_transform_cache {

MAPNIK_DECL void init(std::string const& source, std::string const& dest);
MAPNIK_DECL proj_transform const* get(std::string const& source, std::string const& dest);

} // namespace proj_transform_cache
} // namespace mapnik

#endif // MAPNIK_PROJ_TRANSFORM_CACHE_HPP
