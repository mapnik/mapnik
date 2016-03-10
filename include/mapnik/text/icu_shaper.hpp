/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_ICU_SHAPER_HPP
#define MAPNIK_ICU_SHAPER_HPP

// mapnik
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/text_line.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/text/font_feature_settings.hpp>
#include <mapnik/text/itemizer.hpp>
#include <mapnik/safe_cast.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/debug.hpp>

// stl
#include <list>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>
#include <unicode/ushape.h>
#include <unicode/schriter.h>
#pragma GCC diagnostic pop

namespace mapnik
{

struct icu_shaper
{
static void shape_text(text_line & line,
                       text_itemizer & itemizer,
                       std::map<unsigned,double> & width_map,
                       face_manager_freetype & font_manager,
                       double scale_factor )
{
    unsigned start = line.first_char();
    unsigned end = line.last_char();
    size_t length = end - start;
    if (!length) return;
    line.reserve(length);
    std::list<text_item> const& list = itemizer.itemize(start, end);
    mapnik::value_unicode_string const& text = itemizer.text();
    UErrorCode err = U_ZERO_ERROR;
    mapnik::value_unicode_string shaped;
    mapnik::value_unicode_string reordered;

    for (auto const& text_item : list)
    {
        face_set_ptr face_set = font_manager.get_face_set(text_item.format_->face_name, text_item.format_->fontset);
        double size = text_item.format_->text_size * scale_factor;
        face_set->set_unscaled_character_sizes();
        for (auto const& face : *face_set)
        {

            UBiDi *bidi = ubidi_openSized(length, 0, &err);
            ubidi_setPara(bidi, text.getBuffer() + start, length, UBIDI_DEFAULT_LTR, 0, &err);
            ubidi_writeReordered(bidi, reordered.getBuffer(length),
                                 length, UBIDI_DO_MIRRORING, &err);
            ubidi_close(bidi);
            reordered.releaseBuffer(length);

            int32_t num_char = u_shapeArabic(reordered.getBuffer(), length,
                                             shaped.getBuffer(length), length,
                                             U_SHAPE_LETTERS_SHAPE | U_SHAPE_LENGTH_FIXED_SPACES_NEAR |
                                             U_SHAPE_TEXT_DIRECTION_VISUAL_LTR, &err);
            if (num_char < 0)
            {
                MAPNIK_LOG_ERROR(icu_shaper) << " u_shapeArabic returned negative num_char " << num_char;
            }
            std::size_t num_chars = static_cast<std::size_t>(num_char);
            shaped.releaseBuffer(length);
            bool shaped_status = true;
            double max_glyph_height = 0;
            if (U_SUCCESS(err) && (num_chars == length))
            {
                unsigned char_index = 0;
                U_NAMESPACE_QUALIFIER StringCharacterIterator iter(shaped);
                for (iter.setToStart(); iter.hasNext();)
                {
                    UChar ch = iter.nextPostInc();
                    auto codepoint = FT_Get_Char_Index(face->get_face(), ch);
                    glyph_info g(codepoint,char_index,text_item.format_);
                    //g.offset.clear();
                    if (g.glyph_index == 0)
                    {
                        shaped_status = false;
                        break;
                    }
                    if (face->glyph_dimensions(g))
                    {
                        g.face = face;
                        g.scale_multiplier = size / face->get_face()->units_per_EM;
                        double tmp_height = g.height();
                        if (tmp_height > max_glyph_height) max_glyph_height = tmp_height;
                        width_map[char_index++] += g.advance();
                        line.add_glyph(std::move(g), scale_factor);
                    }
                }
            }
            if (!shaped_status) continue;
            line.update_max_char_height(max_glyph_height);
            return;
        }
    }
}

};
} //namespace mapnik

#endif // MAPNIK_ICU_SHAPER_HPP
