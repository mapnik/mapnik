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
#include <mapnik/polygon_pattern_symbolizer.hpp>

namespace mapnik
{

static const char * pattern_alignment_strings[] = {
    "local", // feature
    "global", // map
    ""
};

IMPLEMENT_ENUM( pattern_alignment_e, pattern_alignment_strings )

polygon_pattern_symbolizer::polygon_pattern_symbolizer(path_expression_ptr file)
: symbolizer_with_image(file), symbolizer_base(),
    alignment_(LOCAL_ALIGNMENT),
    gamma_(1.0),
    gamma_method_(GAMMA_POWER) {}

polygon_pattern_symbolizer::polygon_pattern_symbolizer(polygon_pattern_symbolizer const& rhs)
    : symbolizer_with_image(rhs), symbolizer_base(rhs),
      alignment_(rhs.alignment_),
      gamma_(rhs.gamma_),
      gamma_method_(rhs.gamma_method_) {}

pattern_alignment_e polygon_pattern_symbolizer::get_alignment() const
{
    return alignment_;
}

void polygon_pattern_symbolizer::set_alignment(pattern_alignment_e align)
{
    alignment_ = align;
}

double polygon_pattern_symbolizer::get_gamma() const
{
    return gamma_;
}

void polygon_pattern_symbolizer::set_gamma(double gamma)
{
    gamma_ = gamma;
}

void polygon_pattern_symbolizer::set_gamma_method(gamma_method_e gamma_method)
{
    gamma_method_ = gamma_method;
}

gamma_method_e polygon_pattern_symbolizer::get_gamma_method() const
{
    return gamma_method_;
}

}

