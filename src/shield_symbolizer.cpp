/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
 * Copyright (C) 2006 10East Corp.
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
// boost
#include <boost/scoped_ptr.hpp>
// mapnik
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_reader.hpp>

namespace mapnik
{
    shield_symbolizer::shield_symbolizer()
        : symbol_(new ImageData32(4,4)),
          overlap_(false)
    {
        //default point symbol is black 4x4px square
        symbol_->set(0xff000000);
    }
    
    shield_symbolizer::shield_symbolizer(
                          std::string const& name,
                          unsigned size,
                          Color const& fill, 
                          std::string const& file,
                          std::string const& type,
                          unsigned width,unsigned height)
        : name_(name), size_(size), fill_(fill), symbol_(new ImageData32(width,height))
    {
        try 
        {
            boost::scoped_ptr<ImageReader> reader(get_image_reader(type,file));
            if (reader.get())
            {
                reader->read(0,0,*symbol_);		
            }
        } 
        catch (...) 
        {
            std::clog<<"exception caught..." << std::endl;
        }
    }
    
    shield_symbolizer::shield_symbolizer(shield_symbolizer const& rhs)
        : name_(rhs.name_),
          size_(rhs.size_),
          fill_(rhs.fill_),
          symbol_(rhs.symbol_),
          overlap_(rhs.overlap_)
    {}
    
    void shield_symbolizer::set_data( boost::shared_ptr<ImageData32> symbol)
    {
        symbol_ = symbol;
    }

    boost::shared_ptr<ImageData32> const& shield_symbolizer::get_data() const
    {
        return symbol_;
    }
    
    std::string const& shield_symbolizer::get_name() const
    {
      return name_;
    }
    
    void shield_symbolizer::set_allow_overlap(bool overlap)
    {
        overlap_ = overlap;
    }
    
    bool shield_symbolizer::get_allow_overlap() const
    {
        return overlap_;
    }

    unsigned shield_symbolizer::get_text_size() const
    {
        return size_;
    }

    Color const& shield_symbolizer::get_fill() const
    {
        return fill_;
    }
}

