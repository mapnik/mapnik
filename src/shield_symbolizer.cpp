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
    shield_symbolizer::shield_symbolizer(
                          std::string const& name,
                          std::string const& face_name,
                          unsigned size,
                          color const& fill,
                          std::string const& file,
                          std::string const& type,
                          unsigned width,unsigned height)
        : text_symbolizer(name, face_name, size, fill),
          symbolizer_with_image( file, type, width, height ),
          unlock_image_(false),
          no_text_(false)
    {
    }

    shield_symbolizer::shield_symbolizer(
                          std::string const& name,
                          unsigned size,
                          color const& fill,
                          std::string const& file,
                          std::string const& type,
                          unsigned width,unsigned height)
        : text_symbolizer(name, size, fill),
          symbolizer_with_image( file, type, width, height ),
          unlock_image_(false),
          no_text_(false)
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

    void shield_symbolizer::set_no_text(bool no_text)
    {
       no_text_ = no_text;
    }

    bool shield_symbolizer::get_no_text() const
    {
      return no_text_;
    }
}

