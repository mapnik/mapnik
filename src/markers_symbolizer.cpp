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

// mapnik
#include <mapnik/markers_symbolizer.hpp>

// boost
#include <boost/make_shared.hpp>

namespace mapnik {

static const char * marker_placement_strings[] = {
    "point",
    "interior",
    "line",
    ""
};

IMPLEMENT_ENUM( marker_placement_e, marker_placement_strings )

markers_symbolizer::markers_symbolizer()
    : symbolizer_with_image(parse_path("shape://ellipse")),
      symbolizer_base(),
      width_(),
      height_(),
      ignore_placement_(false),
      allow_overlap_(false),
      spacing_(100.0),
      max_error_(0.2),
      marker_p_(MARKER_POINT_PLACEMENT) { }

markers_symbolizer::markers_symbolizer(path_expression_ptr const& filename)
    : symbolizer_with_image(filename),
      symbolizer_base(),
      width_(),
      height_(),
      ignore_placement_(false),
      allow_overlap_(false),
      spacing_(100.0),
      max_error_(0.2),
      marker_p_(MARKER_POINT_PLACEMENT) { }

markers_symbolizer::markers_symbolizer(markers_symbolizer const& rhs)
    : symbolizer_with_image(rhs),
      symbolizer_base(rhs),
      width_(rhs.width_),
      height_(rhs.height_),
      ignore_placement_(rhs.ignore_placement_),
      allow_overlap_(rhs.allow_overlap_),
      spacing_(rhs.spacing_),
      max_error_(rhs.max_error_),
      fill_(rhs.fill_),
      fill_opacity_(rhs.fill_opacity_),
      stroke_(rhs.stroke_),
      marker_p_(rhs.marker_p_) {}

void markers_symbolizer::set_ignore_placement(bool ignore_placement)
{
    ignore_placement_ = ignore_placement;
}

bool markers_symbolizer::get_ignore_placement() const
{
    return ignore_placement_;
}

void markers_symbolizer::set_allow_overlap(bool overlap)
{
    allow_overlap_ = overlap;
}

bool markers_symbolizer::get_allow_overlap() const
{
    return allow_overlap_;
}

void markers_symbolizer::set_spacing(double spacing)
{
    spacing_ = spacing;
}

double markers_symbolizer::get_spacing() const
{
    return spacing_;
}

void markers_symbolizer::set_max_error(double max_error)
{
    max_error_ = max_error;
}

double markers_symbolizer::get_max_error() const
{
    return max_error_;
}

void markers_symbolizer::set_fill(color const& fill)
{
    fill_ = fill;
}

boost::optional<color> markers_symbolizer::get_fill() const
{
    return fill_;
}

void markers_symbolizer::set_fill_opacity(float opacity)
{
    fill_opacity_ = opacity;
}

boost::optional<float> markers_symbolizer::get_fill_opacity() const
{
    return fill_opacity_;
}

void markers_symbolizer::set_width(expression_ptr const& width)
{
    width_ = width;
}

expression_ptr const& markers_symbolizer::get_width() const
{
    return width_;
}

void markers_symbolizer::set_height(expression_ptr const& height)
{
    height_ = height;
}

expression_ptr const& markers_symbolizer::get_height() const
{
    return height_;
}

boost::optional<stroke> markers_symbolizer::get_stroke() const
{
    return stroke_;
}

void markers_symbolizer::set_stroke(stroke const& stroke)
{
    stroke_ = stroke;
}

void markers_symbolizer::set_marker_placement(marker_placement_e marker_p)
{
    marker_p_ = marker_p;
}

marker_placement_e markers_symbolizer::get_marker_placement() const
{
    return marker_p_;
}

}
