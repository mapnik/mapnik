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
#include <pdf/pdf_line_pattern_symbolizer.hpp>

// mapnik
#include <mapnik/line_pattern_symbolizer.hpp>
#include <mapnik/image_reader.hpp>

// stl
#include <iostream>

namespace mapnik
{

  pdf_line_pattern_symbolizer::pdf_line_pattern_symbolizer(std::string const& file, std::string const& type, const double line_width, const unsigned image_width, const unsigned image_height, const double _output_width, const double _output_height)
    : line_pattern_symbolizer(file, type, image_width, image_height),
      line_stroke(Color(), line_width)
  {
    output_width = _output_width;
    output_height = _output_height;
  }

  pdf_line_pattern_symbolizer::pdf_line_pattern_symbolizer(pdf_line_pattern_symbolizer const& rhs)
    : line_pattern_symbolizer(rhs)
  {
    output_width = rhs.output_width;
    output_height = rhs.output_height;
    line_stroke = rhs.line_stroke;
  }

  pdf_line_pattern_symbolizer::pdf_line_pattern_symbolizer(line_pattern_symbolizer const& rhs, const double line_width, const double _output_width, const double _output_height)
    : line_pattern_symbolizer(rhs),
      line_stroke(Color(), line_width)
  {
    output_width = _output_width;
    output_height = _output_height;
  }


  void pdf_line_pattern_symbolizer::set_output_size(const double width, const double height)
  {
    output_width = width;
    output_height = height;
  }


} //namespace mapnik

#endif //ENABLE_PDF
