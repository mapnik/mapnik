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

#ifndef MAPNIK_SSE_HPP
#define MAPNIK_SSE_HPP

#include <emmintrin.h>
#include <xmmintrin.h>

#define ROUND_DOWN(x, s) ((x) & ~((s)-1))

typedef union 
{
    __m128i v;
    int32_t i32[4];
    uint32_t u32[4];
    uint16_t u16[8];
    uint8_t u8[16];
} m128_int;

#endif // MAPNIK_SSE_HPP
