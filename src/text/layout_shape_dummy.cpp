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

// mapnik
#include <mapnik/text/layout.hpp>
#include <mapnik/text/text_properties.hpp>

// stl
#include <list>

// ICU
#include <unicode/unistr.h>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
}

namespace mapnik
{
/* Dummy shaping limitations:
 * - LTR text only
 * - Only first font in fontset is used
 * - No kerning
 * - No complex scripts
 */

void text_layout::shape_text(text_line_ptr line)
{
    unsigned start = line->get_first_char();
    unsigned end = line->get_last_char();
    UnicodeString const& text = itemizer_.get_text();

    size_t length = end - start;

    line->reserve(length); //Preallocate memory

    std::list<text_item> const& list = itemizer_.itemize(start, end);
    std::list<text_item>::const_iterator itr = list.begin(), list_end = list.end();
    for (; itr != list_end; itr++)
    {
        face_set_ptr face_set = font_manager_.get_face_set(itr->format->face_name, itr->format->fontset);
        face_set->set_character_sizes(itr->format->text_size * scale_factor_);
        if (face_set->begin() == face_set->end()) return; //Invalid face set
        face_ptr face = *(face_set->begin());
        FT_Face freetype_face = face->get_face();

        for (int i = itr->start; i < itr->end; i++)
        {
            UChar32 c = text.char32At(i);
            glyph_info tmp;
            tmp.char_index = i;
            tmp.glyph_index = FT_Get_Char_Index(freetype_face, c);
            if (tmp.glyph_index == 0) continue; //Skip unknown characters
            tmp.width = 0; //Filled in by glyph_dimensions
            tmp.offset.clear();
            tmp.face = face;
            tmp.format = itr->format;
            face->glyph_dimensions(tmp);

            width_map_[i] += tmp.width;

            line->add_glyph(tmp, scale_factor_);
        }
        line->update_max_char_height(face->get_char_height());
    }
}

} //ns mapnik

