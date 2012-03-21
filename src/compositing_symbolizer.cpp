/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
// mapnik
#include <mapnik/compositing_symbolizer.hpp>

namespace mapnik
{

compositing_symbolizer::compositing_symbolizer()
    : symbolizer_base(),
      fill_(color(128,128,128)),
      opacity_(1.0),
      gamma_(1.0),
      gamma_method_(GAMMA_POWER),
      smooth_(0.0),
      comp_op_(clear) {}

compositing_symbolizer::compositing_symbolizer(color const& fill)
    : symbolizer_base(),
      fill_(fill),
      opacity_(1.0),
      gamma_(1.0),
      gamma_method_(GAMMA_POWER),
      smooth_(0.0),
      comp_op_(clear) {}

color const& compositing_symbolizer::get_fill() const
{
    return fill_;
}

void compositing_symbolizer::set_fill(color const& fill)
{
    fill_ = fill;
}

void compositing_symbolizer::set_opacity(double opacity)
{
    opacity_ = opacity;
}

double compositing_symbolizer::get_opacity() const
{
    return opacity_;
}

void compositing_symbolizer::set_gamma(double gamma)
{
    gamma_ = gamma;
}

double compositing_symbolizer::get_gamma() const
{
    return gamma_;
}

void compositing_symbolizer::set_gamma_method(gamma_method_e gamma_method)
{
    gamma_method_ = gamma_method;
}

gamma_method_e compositing_symbolizer::get_gamma_method() const
{
    return gamma_method_;
}

void compositing_symbolizer::set_smooth(double smooth)
{
    smooth_ = smooth;
}

double compositing_symbolizer::smooth() const
{
    return smooth_;
}

void compositing_symbolizer::set_comp_op(composite_mode_e comp_op)
{
    comp_op_ = comp_op;
}

composite_mode_e compositing_symbolizer::comp_op() const
{
    return comp_op_;
}

}
