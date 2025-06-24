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

#ifndef MAPNIK_SVG_PATH_ATTRIBUTES_HPP
#define MAPNIK_SVG_PATH_ATTRIBUTES_HPP

// mapnik
#include <mapnik/gradient.hpp>
#include <mapnik/symbolizer_base.hpp> // dash_array

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_math_stroke.h"
#include "agg_color_rgba.h"
#include "agg_trans_affine.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace svg {

struct path_attributes
{
    mapnik::gradient fill_gradient;
    mapnik::gradient stroke_gradient;
    agg::trans_affine transform;
    double opacity;
    double fill_opacity;
    double stroke_opacity;
    double miter_limit;
    double stroke_width;
    unsigned index;
    agg::rgba8 fill_color;
    agg::rgba8 stroke_color;
    agg::line_join_e line_join;
    agg::line_cap_e line_cap;
    bool fill_flag;
    bool fill_none;
    bool stroke_flag;
    bool stroke_none;
    bool even_odd_flag;
    bool visibility_flag;
    bool display_flag;
    dash_array dash;
    double dash_offset;
    // Empty constructor
    path_attributes()
        : fill_gradient()
        , stroke_gradient()
        , transform()
        , opacity(1.0)
        , fill_opacity(1.0)
        , stroke_opacity(1.0)
        , miter_limit(4.0)
        , stroke_width(1.0)
        , index(0)
        , fill_color(agg::rgba(0, 0, 0))
        , stroke_color(agg::rgba(0, 0, 0))
        , line_join(agg::miter_join)
        , line_cap(agg::butt_cap)
        , fill_flag(true)
        , fill_none(false)
        , stroke_flag(false)
        , stroke_none(false)
        , even_odd_flag(false)
        , visibility_flag(true)
        , display_flag(true)
        , dash()
        , dash_offset(0.0)
    {}

    // Copy constructor
    path_attributes(path_attributes const& attr)
        : fill_gradient(attr.fill_gradient)
        , stroke_gradient(attr.stroke_gradient)
        , transform(attr.transform)
        , opacity(attr.opacity)
        , fill_opacity(attr.fill_opacity)
        , stroke_opacity(attr.stroke_opacity)
        , miter_limit(attr.miter_limit)
        , stroke_width(attr.stroke_width)
        , index(attr.index)
        , fill_color(attr.fill_color)
        , stroke_color(attr.stroke_color)
        , line_join(attr.line_join)
        , line_cap(attr.line_cap)
        , fill_flag(attr.fill_flag)
        , fill_none(attr.fill_none)
        , stroke_flag(attr.stroke_flag)
        , stroke_none(attr.stroke_none)
        , even_odd_flag(attr.even_odd_flag)
        , visibility_flag(attr.visibility_flag)
        , display_flag(attr.display_flag)
        , dash(attr.dash)
        , dash_offset(attr.dash_offset)
    {}
    // Copy constructor with new index value
    path_attributes(path_attributes const& attr, unsigned idx)
        : fill_gradient(attr.fill_gradient)
        , stroke_gradient(attr.stroke_gradient)
        , transform(attr.transform)
        , opacity(attr.opacity)
        , fill_opacity(attr.fill_opacity)
        , stroke_opacity(attr.stroke_opacity)
        , miter_limit(attr.miter_limit)
        , stroke_width(attr.stroke_width)
        , index(idx)
        , fill_color(attr.fill_color)
        , stroke_color(attr.stroke_color)
        , line_join(attr.line_join)
        , line_cap(attr.line_cap)
        , fill_flag(attr.fill_flag)
        , fill_none(attr.fill_none)
        , stroke_flag(attr.stroke_flag)
        , stroke_none(attr.stroke_none)
        , even_odd_flag(attr.even_odd_flag)
        , visibility_flag(attr.visibility_flag)
        , display_flag(attr.display_flag)
        , dash(attr.dash)
        , dash_offset(attr.dash_offset)
    {}
};

} // namespace svg
} // namespace mapnik

#endif // MAPNIK_SVG_PATH_ATTRIBUTES_HPP
