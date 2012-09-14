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
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/enumeration.hpp>

namespace mapnik
{

static const char * point_placement_strings[] = {
    "centroid",
    "interior",
    ""
};

IMPLEMENT_ENUM( point_placement_e, point_placement_strings )

point_symbolizer::point_symbolizer()
: symbolizer_with_image(path_expression_ptr(new path_expression)), // FIXME
    symbolizer_base(),
    overlap_(false),
    point_p_(CENTROID_POINT_PLACEMENT),
    ignore_placement_(false) {}

point_symbolizer::point_symbolizer(path_expression_ptr file)
    : symbolizer_with_image(file),
      symbolizer_base(),
      overlap_(false),
      point_p_(CENTROID_POINT_PLACEMENT),
      ignore_placement_(false) {}

point_symbolizer::point_symbolizer(point_symbolizer const& rhs)
    : symbolizer_with_image(rhs),
      symbolizer_base(rhs),
      overlap_(rhs.overlap_),
      point_p_(rhs.point_p_),
      ignore_placement_(rhs.ignore_placement_) {}

void point_symbolizer::set_allow_overlap(bool overlap)
{
    overlap_ = overlap;
}

bool point_symbolizer::get_allow_overlap() const
{
    return overlap_;
}

void point_symbolizer::set_point_placement(point_placement_e point_p)
{
    point_p_ = point_p;
}

point_placement_e point_symbolizer::get_point_placement() const
{
    return point_p_;
}

void point_symbolizer::set_ignore_placement(bool ignore_placement)
{
    ignore_placement_ = ignore_placement;
}

bool point_symbolizer::get_ignore_placement() const
{
    return ignore_placement_;
}

}

