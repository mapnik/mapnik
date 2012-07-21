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
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_reader.hpp>

// boost
#include <boost/scoped_ptr.hpp>

// stl
#include <iostream>

namespace mapnik
{

shield_symbolizer::shield_symbolizer(text_placements_ptr placements)
    : text_symbolizer(placements),
      symbolizer_with_image(),
      unlock_image_(false),
      shield_displacement_(0,0)
{
}

shield_symbolizer::shield_symbolizer(
    expression_ptr name,
    std::string const& face_name,
    float size,
    color const& fill,
    path_expression_ptr file)
    : text_symbolizer(name, face_name, size, fill),
      symbolizer_with_image(file),
      unlock_image_(false),
      shield_displacement_(0, 0)
{
}

shield_symbolizer::shield_symbolizer(
    expression_ptr name,
    float size,
    color const& fill,
    path_expression_ptr file)
    : text_symbolizer(name, size, fill),
      symbolizer_with_image(file),
      unlock_image_(false),
      shield_displacement_(0, 0)
{
}

void shield_symbolizer::set_unlock_image(bool unlock_image)
{
    unlock_image_ = unlock_image;
}

bool shield_symbolizer::get_unlock_image() const
{
    return unlock_image_;
}

void shield_symbolizer::set_shield_displacement(double shield_dx,double shield_dy)
{
    shield_displacement_ = std::make_pair(shield_dx, shield_dy);
}

position const& shield_symbolizer::get_shield_displacement() const
{
    return shield_displacement_;
}

}
