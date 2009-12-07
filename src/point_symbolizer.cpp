/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
#include <mapnik/point_symbolizer.hpp>

#include <mapnik/image_data.hpp>
#include <mapnik/image_reader.hpp>
// boost
#include <boost/scoped_ptr.hpp>
// stl
#include <iostream>

namespace mapnik
{
point_symbolizer::point_symbolizer()
    : symbolizer_with_image(boost::shared_ptr<ImageData32>(new ImageData32(4,4))),
      opacity_(1.0),
      overlap_(false)
          
{
    //default point symbol is black 4x4px square
    image_->set(0xff000000);
}
    
point_symbolizer::point_symbolizer(std::string const& file,
				   std::string const& type,
				   unsigned width,unsigned height) 
    : symbolizer_with_image(file, type, width, height),
      opacity_(1.0),      
      overlap_(false)
          
{ }
    
point_symbolizer::point_symbolizer(point_symbolizer const& rhs)
    : symbolizer_with_image(rhs),
      opacity_(rhs.opacity_),
      overlap_(rhs.overlap_)
{}
    
void point_symbolizer::set_allow_overlap(bool overlap)
{
    overlap_ = overlap;
}
    
bool point_symbolizer::get_allow_overlap() const
{
    return overlap_;
}

void point_symbolizer::set_opacity(float opacity)
{
    opacity_ = opacity;
}

float point_symbolizer::get_opacity() const
{
    return opacity_;
}

}

