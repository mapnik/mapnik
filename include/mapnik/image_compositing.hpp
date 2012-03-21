/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_IMAGE_COMPOSITING_HPP
#define MAPNIK_IMAGE_COMPOSITING_HPP

// stl
#include <string>

namespace mapnik
{

// Compositing modes
// http://www.w3.org/TR/2009/WD-SVGCompositing-20090430/

enum composite_mode_e
{
    clear = 0,
    src,
    dst,
    src_over,
    dst_over,
    src_in,
    dst_in,
    src_out,
    dst_out,
    src_atop,
    dst_atop,
    _xor,
    plus,
    minus,
    multiply,
    screen,
    overlay,
    darken,
    lighten,
    color_dodge,
    color_burn,
    hard_light,
    soft_light,
    difference,
    exclusion,
    contrast,
    invert,
    invert_rgb
};

composite_mode_e comp_op_from_string(std::string const& name);

template <typename T1, typename T2>
void composite(T1 & im, T2 & im2, composite_mode_e mode);

}
#endif // MAPNIK_IMAGE_COMPOSITING_HPP
