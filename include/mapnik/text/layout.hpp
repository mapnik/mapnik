/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
#ifndef MAPNIK_TEXT_LAYOUT_HPP
#define MAPNIK_TEXT_LAYOUT_HPP

//mapnik
#include <mapnik/text/itemizer.hpp>
#include <mapnik/font_engine_freetype.hpp>

//stl
#include <vector>

namespace mapnik
{

struct glyph_info
{
      font_glyph glyph;
      uint32_t char_index; //Position in the string of all characters i.e. before itemizing
      uint32_t x_advance;
};

class text_layout
{
public:
    text_layout(face_manager_freetype & font_manager);
    inline void add_text(UnicodeString const& str, char_properties const& format)
    {
        itemizer.add_text(str, format);
    }

    void break_lines();
    void shape_text();
    void clear();

private:
    text_itemizer itemizer;
    std::vector<glyph_info> glyphs_;
    face_manager_freetype &font_manager_;
};
}

#endif // TEXT_LAYOUT_HPP
