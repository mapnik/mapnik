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
// stl
#include <iostream>
// mapnik
#include <mapnik/image_reader.hpp>
#include <mapnik/polygon_pattern_symbolizer.hpp>

namespace mapnik
{
    polygon_pattern_symbolizer::polygon_pattern_symbolizer(std::string const& file,
                                                           std::string const& type,
                                                           unsigned width,unsigned height) 
        : pattern_(new ImageData32(width,height))
    {
        try 
        {
            std::auto_ptr<ImageReader> reader(get_image_reader(type,file));
            if (reader.get())
                reader->read(0,0,*pattern_);		
        } 
        catch (...) 
        {
            std::clog << "exception caught...\n";
        }
    }
    polygon_pattern_symbolizer::polygon_pattern_symbolizer(polygon_pattern_symbolizer const& rhs)
        : pattern_(rhs.pattern_) {}
    
    ImageData32 const& polygon_pattern_symbolizer::get_pattern() const
    {
        return *pattern_;
    }
}
