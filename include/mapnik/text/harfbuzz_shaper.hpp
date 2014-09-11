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

#ifndef MAPNIK_HARFBUZZ_SHAPER_HPP
#define MAPNIK_HARFBUZZ_SHAPER_HPP

// mapnik
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/text_line.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/text/font_feature_settings.hpp>

// stl
#include <list>
#include <type_traits>

// harfbuzz
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

namespace mapnik
{

static inline hb_script_t _icu_script_to_script(UScriptCode script)
{
    if (script == USCRIPT_INVALID_CODE) return HB_SCRIPT_INVALID;
	return hb_script_from_string(uscript_getShortName(script), -1);
}

static inline const uint16_t * uchar_to_utf16(const UChar* src)
{
   static_assert(sizeof(UChar) == sizeof(uint16_t),"UChar is eq size to uint16_t");
#if defined(_MSC_VER)
   return reinterpret_cast<const uint16_t *>(src);
#else
   return src;
#endif
}

struct harfbuzz_shaper
{
static void shape_text(text_line & line,
                       text_itemizer & itemizer,
                       std::map<unsigned,double> & width_map,
                       face_manager_freetype & font_manager,
                       double scale_factor)
{
    unsigned start = line.first_char();
    unsigned end = line.last_char();
    size_t length = end - start;
    if (!length) return;
    line.reserve(length);
    std::list<text_item> const& list = itemizer.itemize(start, end);

    auto hb_buffer_deleter = [](hb_buffer_t * buffer) { hb_buffer_destroy(buffer);};
    const std::unique_ptr<hb_buffer_t, decltype(hb_buffer_deleter)> buffer(hb_buffer_create(),hb_buffer_deleter);
    hb_buffer_pre_allocate(buffer.get(), length);
    mapnik::value_unicode_string const& text = itemizer.text();

    font_feature_settings_ptr features = list.front().format->font_feature_settings;

    for (auto const& text_item : list)
    {
        face_set_ptr face_set = font_manager.get_face_set(text_item.format->face_name, text_item.format->fontset);
        double size = text_item.format->text_size * scale_factor;
        face_set->set_unscaled_character_sizes();
        std::size_t num_faces = face_set->size();
        std::size_t pos = 0;
        for (auto const& face : *face_set)
        {
            ++pos;
            hb_buffer_clear_contents(buffer.get());
            hb_buffer_add_utf16(buffer.get(), uchar_to_utf16(text.getBuffer()), text.length(), text_item.start, text_item.end - text_item.start);
            hb_buffer_set_direction(buffer.get(), (text_item.rtl == UBIDI_RTL)?HB_DIRECTION_RTL:HB_DIRECTION_LTR);
            hb_buffer_set_script(buffer.get(), _icu_script_to_script(text_item.script));
            hb_font_t *font(hb_ft_font_create(face->get_face(), nullptr));
            hb_shape(font, buffer.get(), features->get_features(), features->count());
            hb_font_destroy(font);

            unsigned num_glyphs = hb_buffer_get_length(buffer.get());

            hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer.get(), nullptr);
            hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer.get(), nullptr);

            bool font_has_all_glyphs = true;
            // Check if all glyphs are valid.
            for (unsigned i=0; i<num_glyphs; ++i)
            {
                if (!glyphs[i].codepoint)
                {
                    font_has_all_glyphs = false;
                    break;
                }
            }
            if (!font_has_all_glyphs && (pos < num_faces))
            {
                //Try next font in fontset
                continue;
            }

            for (unsigned i=0; i<num_glyphs; ++i)
            {
                glyph_info tmp;
                tmp.glyph_index = glyphs[i].codepoint;
                if (face->glyph_dimensions(tmp))
                {
                    tmp.char_index = glyphs[i].cluster;
                    tmp.face = face;
                    tmp.format = text_item.format;
                    tmp.scale_multiplier = size / face->get_face()->units_per_EM;
                    //Overwrite default advance with better value provided by HarfBuzz
                    tmp.unscaled_advance = positions[i].x_advance;

                    tmp.offset.set(positions[i].x_offset * tmp.scale_multiplier, positions[i].y_offset * tmp.scale_multiplier);
                    width_map[glyphs[i].cluster] += tmp.advance();
                    line.add_glyph(tmp, scale_factor);
                }
            }
            line.update_max_char_height(face->get_char_height(size));
            break; //When we reach this point the current font had all glyphs.
        }
    }
}
};
} // namespace mapnik

#endif // MAPNIK_HARFBUZZ_SHAPER_HPP
