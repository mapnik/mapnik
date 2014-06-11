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
          unscaled_ymin(0.0),
          unscaled_ymax(0.0),
          unscaled_width(0.0),
          unscaled_height(0.0),
          unscaled_advance(0.0),
          unscaled_ascender(0.0),
          unscaled_descender(0.0),
          unscaled_line_height(0.0),
          scale_multiplier(0.0),
          offset(),
          format() {}
    glyph_index_t glyph_index;
    face_ptr face;
    // Position in the string of all characters i.e. before itemizing
    unsigned char_index;
    double unscaled_ymin;
    double unscaled_ymax;
    double unscaled_width;
    double unscaled_height;
    double unscaled_advance;
    double unscaled_ascender;
    double unscaled_descender;
    // Line height returned by freetype, includes normal font
    // line spacing, but not additional user defined spacing
    double unscaled_line_height;
    double scale_multiplier;
    pixel_position offset;
    char_properties_ptr format;

    double ymin() const { return unscaled_ymin * scale_multiplier; }
    double ymax() const { return unscaled_ymax * scale_multiplier; }
    double width() const { return unscaled_width * scale_multiplier; };
    double height() const { return unscaled_height * scale_multiplier; };
    double advance() const { return unscaled_advance * scale_multiplier; };
    double ascender() const { return unscaled_ascender * scale_multiplier; };
    double descender() const { return unscaled_descender * scale_multiplier; };
    double line_height() const { return unscaled_line_height * scale_multiplier; };
};

} //ns mapnik

#endif // GLYPH_INFO_HPP
