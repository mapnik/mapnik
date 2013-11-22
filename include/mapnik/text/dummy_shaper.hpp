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

#ifndef MAPNIK_DUMMY_SHAPER_HPP
#define MAPNIK_DUMMY_SHAPER_HPP

// mapnik
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/text_line.hpp>
#include <mapnik/text/face.hpp>
// stl
#include <list>

namespace mapnik
{

struct dummy_shaper
{

/* Dummy shaping limitations:
 * - LTR text only
 * - Only first font in fontset is used
 * - No kerning
 * - No complex scripts
 */

static void shape_text(text_line & line,
                       text_itemizer & itemizer,
                       std::map<unsigned,double> & width_map,
                       face_manager_freetype & font_manager,
                       double scale_factor )
{
    unsigned start = line.first_char();
    unsigned end = line.last_char();
    mapnik::value_unicode_string const& text = itemizer.text();
    size_t length = end - start;
    if (!length) return;
    line.reserve(length);
    std::list<text_item> const& list = itemizer.itemize(start, end);

    for (auto const& text_item : list)
    {
        face_set_ptr face_set = font_manager.get_face_set(text_item.format->face_name, text_item.format->fontset);
        double size = text_item.format->text_size * scale_factor;
        face_set->set_character_sizes(size);
        if (face_set->begin() == face_set->end()) return; // Invalid face set
        face_ptr face = *(face_set->begin());
        FT_Face freetype_face = face->get_face();

        for (unsigned i = text_item.start; i < text_item.end; ++i)
        {
            UChar32 c = text.char32At(i);
            glyph_info tmp;
            tmp.glyph_index = FT_Get_Char_Index(freetype_face, c);
            if (tmp.glyph_index == 0) continue; // Skip unknown characters
            tmp.char_index = i;
            tmp.width = 0; // Filled in by glyph_dimensions
            tmp.offset.clear();
            tmp.face = face;
            tmp.format = text_item.format;
            face->glyph_dimensions(tmp);
            width_map[i] += tmp.width;
            line.add_glyph(tmp, scale_factor);
        }
        line.update_max_char_height(face->get_char_height());
    }
}

};

} // namespace mapnik

#endif // MAPNIK_DUMMY_SHAPER_HPP
