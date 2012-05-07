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
#include <mapnik/polygon_symbolizer.hpp>

namespace mapnik
{

polygon_symbolizer::polygon_symbolizer()
    : symbolizer_base(),
      fill_(color(128,128,128)),
      opacity_(1.0),
      gamma_(1.0),
      gamma_method_(GAMMA_POWER)
      {}

polygon_symbolizer::polygon_symbolizer(color const& fill)
    : symbolizer_base(),
      fill_(fill),
      opacity_(1.0),
      gamma_(1.0),
      gamma_method_(GAMMA_POWER)
      {}

color const& polygon_symbolizer::get_fill() const
{
    return fill_;
}

void polygon_symbolizer::set_fill(color const& fill)
{
    fill_ = fill;
}

void polygon_symbolizer::set_opacity(double opacity)
{
    opacity_ = opacity;
}

double polygon_symbolizer::get_opacity() const
{
    return opacity_;
}

void polygon_symbolizer::set_gamma(double gamma)
{
    gamma_ = gamma;
}

double polygon_symbolizer::get_gamma() const
{
    return gamma_;
}

void polygon_symbolizer::set_gamma_method(gamma_method_e gamma_method)
{
    gamma_method_ = gamma_method;
}

gamma_method_e polygon_symbolizer::get_gamma_method() const
{
    return gamma_method_;
}

}
