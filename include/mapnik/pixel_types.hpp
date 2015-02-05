/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_PIXEL_TYPES_HPP
#define MAPNIK_PIXEL_TYPES_HPP

#include <mapnik/global.hpp>

struct rgba8_t { using type = std::uint32_t; };
struct gray8_t { using type = std::uint8_t; };
struct gray8s_t { using type = std::int8_t; };
struct gray16_t { using type = std::uint16_t; };
struct gray16s_t { using type = std::int16_t; };
struct gray32_t { using type = std::uint32_t; };
struct gray32s_t { using type = std::int32_t; };
struct gray32f_t { using type = float; };
struct gray64_t { using type = std::uint64_t; };
struct gray64s_t { using type = std::int64_t; };
struct gray64f_t { using type = double; };

#endif // MAPNIK_PIXEL_TYPES_HPP
