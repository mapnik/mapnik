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

#ifndef MAPNIK_IMAGE_COMPOSITING_HPP
#define MAPNIK_IMAGE_COMPOSITING_HPP

#include <mapnik/config.hpp>
#include <mapnik/image.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
#pragma GCC diagnostic pop

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
    invert_rgb,
    grain_merge,
    grain_extract,
    hue,
    saturation,
    _color,
    _value,
    linear_dodge,
    linear_burn,
    divide
};

MAPNIK_DECL boost::optional<composite_mode_e> comp_op_from_string(std::string const& name);
MAPNIK_DECL boost::optional<std::string> comp_op_to_string(composite_mode_e comp_op);

template <typename T>
MAPNIK_DECL void composite(T & dst, T const& src,
                           composite_mode_e mode,
                           float opacity=1,
                           int dx=0,
                           int dy=0);

}
#endif // MAPNIK_IMAGE_COMPOSITING_HPP
