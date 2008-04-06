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

#ifndef PDF_POLYGON_PATTERN_SYMBOLIZER_HPP
#define PDF_POLYGON_PATTERN_SYMBOLIZER_HPP

// mapnik
#include <mapnik/polygon_pattern_symbolizer.hpp>
#include <mapnik/graphics.hpp> 
#include <mapnik/symbolizer.hpp> 

//boost
#include <boost/shared_ptr.hpp>

namespace mapnik
{
  class MAPNIK_DECL pdf_polygon_pattern_symbolizer
    : public polygon_pattern_symbolizer
  {
  public:
    //! \brief constructor
    //!
    //! \param[in] file The image file
    //! \param[in] type The type of the file, e.g. "png"
    //! \param[in] image_width The width of the image in pixels
    //! \param[in] image_height The height of the image in pixels
    //! \param[in] _output_width The scaled (output) width of the image tile (in pdf units)
    //! \param[in] _output_height The scaled (output) height of the image tile (in pdf units)
    pdf_polygon_pattern_symbolizer(std::string const& file,
                               std::string const& type,
                               unsigned image_width, unsigned image_height,
                               double _output_width, double _output_height);

    //! \brief constructor
    //!
    //! Copies the supplied pdf_polygon_pattern_symbolizer
    pdf_polygon_pattern_symbolizer(pdf_polygon_pattern_symbolizer const& rhs);

    //! \brief constructor
    //!
    //! Takes a polygon_pattern_symbolizer and turns it into a pdf_polygon_pattern_symbolizer
    //!
    //! \param[in] rhs The polygon_pattern_symbolizer to start with
    //! \param[in] _output_width The width of the image tile (in pdf units)
    //! \param[in] _output_height The height of the image tile (in pdf units)
    pdf_polygon_pattern_symbolizer(polygon_pattern_symbolizer const& rhs, const double _output_width, const double _output_height);

    //! \brief Sets the output size of the tile
    //!
    //! \param[in] width Width in pdf units
    //! \param[in] height Height in pdf units
    void set_output_size(const double width, const double height);

    //! \brief Gets the output width of the tile
    //!
    //! \return The output width of the image in pdf units
    double get_output_width(void) const { return output_width; };

    //! \brief Gets the output height of the tile
    //!
    //! \return The output height of the image in pdf units
    double get_output_height(void) const { return output_height; };

  private:
    double output_width;    //!< output width of pattern tile in pdf units
    double output_height;   //!< output height of pattern tile in pdf units
  };
}

#endif //PDF_POLYGON_PATTERN_SYMBOLIZER_HPP
