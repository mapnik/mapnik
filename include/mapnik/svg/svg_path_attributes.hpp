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

#ifndef MAPNIK_SVG_PATH_ATTRIBUTES_HPP
#define MAPNIK_SVG_PATH_ATTRIBUTES_HPP

// agg
#include "agg_math_stroke.h"
#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"
#include "agg_trans_affine.h"

// mapnik
#include <mapnik/gradient.hpp>

namespace mapnik {
namespace svg {

struct path_attributes
{
    unsigned     index;
    agg::rgba8   fill_color;
    double       fill_opacity;
    agg::rgba8   stroke_color;
    double       stroke_opacity;
    bool         fill_flag;
    bool         stroke_flag;
    bool         even_odd_flag;
    bool         visibility_flag;
    bool         display_flag;
    agg::line_join_e  line_join;
    agg::line_cap_e   line_cap;
    double       miter_limit;
    double       stroke_width;
    agg::trans_affine transform;
    mapnik::gradient fill_gradient;
    mapnik::gradient stroke_gradient;

    // Empty constructor
    path_attributes() :
        index(0),
        fill_color(agg::rgba(0,0,0)),
        fill_opacity(1.0),
        stroke_color(agg::rgba(0,0,0)),
        stroke_opacity(1.0),
        fill_flag(true),
        stroke_flag(false),
        even_odd_flag(false),
        visibility_flag(true),
        display_flag(true),
        line_join(agg::miter_join),
        line_cap(agg::butt_cap),
        miter_limit(4.0),
        stroke_width(1.0),
        transform(),
        fill_gradient(),
        stroke_gradient()
    {
    }

    // Copy constructor
    path_attributes(const path_attributes& attr)
        : index(attr.index),
          fill_color(attr.fill_color),
          fill_opacity(attr.fill_opacity),
          stroke_color(attr.stroke_color),
          stroke_opacity(attr.stroke_opacity),
          fill_flag(attr.fill_flag),
          stroke_flag(attr.stroke_flag),
          even_odd_flag(attr.even_odd_flag),
          visibility_flag(attr.visibility_flag),
          display_flag(attr.display_flag),
          line_join(attr.line_join),
          line_cap(attr.line_cap),
          miter_limit(attr.miter_limit),
          stroke_width(attr.stroke_width),
          transform(attr.transform),
          fill_gradient(attr.fill_gradient),
          stroke_gradient(attr.stroke_gradient)
    {}

    // Copy constructor with new index value
    path_attributes(path_attributes const& attr, unsigned idx)
        : index(idx),
          fill_color(attr.fill_color),
          fill_opacity(attr.fill_opacity),
          stroke_color(attr.stroke_color),
          stroke_opacity(attr.stroke_opacity),
          fill_flag(attr.fill_flag),
          stroke_flag(attr.stroke_flag),
          even_odd_flag(attr.even_odd_flag),
          visibility_flag(attr.visibility_flag),
          display_flag(attr.display_flag),
          line_join(attr.line_join),
          line_cap(attr.line_cap),
          miter_limit(attr.miter_limit),
          stroke_width(attr.stroke_width),
          transform(attr.transform),
          fill_gradient(attr.fill_gradient),
          stroke_gradient(attr.stroke_gradient)
    {}
};

}}

#endif // MAPNIK_SVG_PATH_ATTRIBUTES_HPP
