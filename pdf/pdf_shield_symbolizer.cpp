/*****************************************************************************
 * 
 * This file is part of the wxPdfDoc modifications for mapnik
 *
 * Copyright (C) 2007 Ben Moores
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

#if ENABLE_PDF

// pdf
#include <pdf/pdf_shield_symbolizer.hpp>

// mapnik
#include <mapnik/image_data.hpp>
#include <mapnik/image_reader.hpp>
// boost
#include <boost/scoped_ptr.hpp>
// stl
#include <iostream>

namespace mapnik
{
  
  pdf_shield_symbolizer::pdf_shield_symbolizer(std::string const& name,
                          std::string const& face_name,
                          unsigned size,
                          Color const& fill, 
                          std::string const& file,
                          std::string const& type,
                          unsigned image_width, unsigned image_height,
                          double _output_width, double _output_height)
    : shield_symbolizer(name, face_name, size, fill, file, type, image_width, image_height)
  {
    output_width = _output_width;
    output_height = _output_height;
  }


  pdf_shield_symbolizer::pdf_shield_symbolizer(pdf_shield_symbolizer const& rhs)
    : shield_symbolizer(rhs)
  {
    output_width = rhs.get_output_width();
    output_height = rhs.get_output_height();
  }

  pdf_shield_symbolizer::pdf_shield_symbolizer(shield_symbolizer const& rhs, const double _output_width, const double _output_height)
    : shield_symbolizer(rhs)
  {
    output_width = _output_width;
    output_height = _output_height;
  }

  void pdf_shield_symbolizer::set_output_size(const double width, const double height)
  {
    output_width = width;
    output_height = height;
  }

}

#endif //ENABLE_PDF
