/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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

//$Id$

#include <mapnik/markers_symbolizer.hpp>

namespace mapnik {

static const char * marker_placement_strings[] = {
    "point",
    "line",
    ""
};

IMPLEMENT_ENUM( marker_placement_e, marker_placement_strings )


static const char * marker_type_strings[] = {
    "arrow",
    "ellipse",
    ""
};

IMPLEMENT_ENUM( marker_type_e, marker_type_strings )

markers_symbolizer::markers_symbolizer()
    : symbolizer_with_image(path_expression_ptr(new path_expression)),
      symbolizer_base(),
      allow_overlap_(false),
      fill_(color(0,0,255)), 
      spacing_(100.0), 
      max_error_(0.2),
      width_(5.0),
      height_(5.0),
      stroke_(),
      marker_p_(MARKER_LINE_PLACEMENT),
      marker_type_(ARROW) {}
    
markers_symbolizer::markers_symbolizer(path_expression_ptr filename) 
    : symbolizer_with_image(filename), 
      symbolizer_base(),
      allow_overlap_(false),
      fill_(color(0,0,255)), 
      spacing_(100.0), 
      max_error_(0.2),
      width_(5.0),
      height_(5.0),
      stroke_(),
      marker_p_(MARKER_LINE_PLACEMENT),
      marker_type_(ARROW) {}

markers_symbolizer::markers_symbolizer(markers_symbolizer const& rhs) 
    : symbolizer_with_image(rhs), 
      symbolizer_base(rhs),
      allow_overlap_(rhs.allow_overlap_),
      fill_(rhs.fill_), 
      spacing_(rhs.spacing_), 
      max_error_(rhs.max_error_),
      width_(rhs.width_),
      height_(rhs.height_),
      stroke_(rhs.stroke_),
      marker_p_(rhs.marker_p_),
      marker_type_(rhs.marker_type_) {}
    
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
    
void markers_symbolizer::set_fill(color fill)
{
    fill_ = fill;
}
    
color const& markers_symbolizer::get_fill() const
{
    return fill_;
}

void markers_symbolizer::set_width(double width)
{
    width_ = width;
}

double markers_symbolizer::get_width() const
{
    return width_;
}

void markers_symbolizer::set_height(double height)
{
    height_ = height;
}

double markers_symbolizer::get_height() const
{
    return height_;
}

stroke const& markers_symbolizer::get_stroke() const
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

void markers_symbolizer::set_marker_type(marker_type_e marker_type)
{
    marker_type_ = marker_type;
}

marker_type_e markers_symbolizer::get_marker_type() const
{
    return marker_type_;
}


}
