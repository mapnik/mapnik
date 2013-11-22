/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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
#ifndef MAPNIK_GLYPH_INFO_HPP
#define MAPNIK_GLYPH_INFO_HPP

//mapnik
#include <mapnik/text/char_properties_ptr.hpp>
#include <mapnik/pixel_position.hpp>

#include <memory>

namespace mapnik
{

class font_face;
typedef std::shared_ptr<font_face> face_ptr;

typedef unsigned glyph_index_t;

struct glyph_info
{
    glyph_info()
        : glyph_index(0),
          face(nullptr),
          char_index(0),
          width(0.0),
          ymin(0.0),
          ymax(0.0),
          line_height(0.0),
          offset(),
          format() {}
    glyph_index_t glyph_index;
    face_ptr face;
    // Position in the string of all characters i.e. before itemizing
    unsigned char_index;
    double width;
    double ymin;
    double ymax;
    // Line height returned by freetype, includes normal font
    // line spacing, but not additional user defined spacing
    double line_height;
    pixel_position offset;
    char_properties_ptr format;
    double height() const { return ymax-ymin; }
};

} //ns mapnik

#endif // GLYPH_INFO_HPP
