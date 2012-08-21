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
#include <mapnik/building_symbolizer.hpp>

namespace mapnik
{

building_symbolizer::building_symbolizer()
    : symbolizer_base(),
      fill_(color(128,128,128)),
      opacity_(1.0)
{}

building_symbolizer::building_symbolizer(color const& fill, expression_ptr const& height)
    : symbolizer_base(),
      fill_(fill),
      height_(height),
      opacity_(1.0) {}

color const& building_symbolizer::get_fill() const
{
    return fill_;
}

void building_symbolizer::set_fill(color const& fill)
{
    fill_ = fill;
}
expression_ptr const& building_symbolizer::height() const
{
    return height_;
}

void building_symbolizer::set_height(expression_ptr const& height)
{
    height_=height;
}

void building_symbolizer::set_opacity(double opacity)
{
    opacity_ = opacity;
}

double building_symbolizer::get_opacity() const
{
    return opacity_;
}

}
