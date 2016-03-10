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

#ifndef MAPNIK_HARFBUZZ_SHAPER_HPP
#define MAPNIK_HARFBUZZ_SHAPER_HPP

// mapnik
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/text_line.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/text/font_feature_settings.hpp>
#include <mapnik/text/itemizer.hpp>
#include <mapnik/safe_cast.hpp>
#include <mapnik/font_engine_freetype.hpp>

// stl
#include <list>
#include <type_traits>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <unicode/uscript.h>
#pragma GCC diagnostic pop

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
    std::size_t length = end - start;
    if (!length) return;

    std::list<text_item> const& list = itemizer.itemize(start, end);

    line.reserve(length);

    auto hb_buffer_deleter = [](hb_buffer_t * buffer) { hb_buffer_destroy(buffer);};
    const std::unique_ptr<hb_buffer_t, decltype(hb_buffer_deleter)> buffer(hb_buffer_create(),hb_buffer_deleter);
    hb_buffer_pre_allocate(buffer.get(), safe_cast<int>(length));
    mapnik::value_unicode_string const& text = itemizer.text();

    for (auto const& text_item : list)
    {
        face_set_ptr face_set = font_manager.get_face_set(text_item.format_->face_name, text_item.format_->fontset);
        double size = text_item.format_->text_size * scale_factor;
        face_set->set_unscaled_character_sizes();
        std::size_t num_faces = face_set->size();
        std::size_t pos = 0;
        font_feature_settings const& ff_settings = text_item.format_->ff_settings;
        int ff_count = safe_cast<int>(ff_settings.count());

        // rendering information for a single glyph
        struct glyph_face_info
        {
            face_ptr face;
            hb_glyph_info_t glyph;
            hb_glyph_position_t position;
        };
        // this table is filled with information for rendering each glyph, so that 
        // several font faces can be used in a single text_item
        std::vector<glyph_face_info> glyphinfos;
        unsigned valid_glyphs = 0;

        for (auto const& face : *face_set)
        {
            ++pos;
            hb_buffer_clear_contents(buffer.get());
            hb_buffer_add_utf16(buffer.get(), uchar_to_utf16(text.getBuffer()), text.length(), text_item.start, static_cast<int>(text_item.end - text_item.start));
            hb_buffer_set_direction(buffer.get(), (text_item.dir == UBIDI_RTL)?HB_DIRECTION_RTL:HB_DIRECTION_LTR);
            hb_buffer_set_script(buffer.get(), _icu_script_to_script(text_item.script));
            hb_font_t *font(hb_ft_font_create(face->get_face(), nullptr));
            // https://github.com/mapnik/test-data-visual/pull/25
            #if HB_VERSION_MAJOR > 0
             #if HB_VERSION_ATLEAST(1, 0 , 5)
            hb_ft_font_set_load_flags(font,FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);
             #endif
            #endif
            hb_shape(font, buffer.get(), ff_settings.get_features(), ff_count);
            hb_font_destroy(font);

            unsigned num_glyphs = hb_buffer_get_length(buffer.get());

            // if the number of rendered glyphs has increased, we need to resize the table 
            if (num_glyphs > glyphinfos.size())
            {
                glyphinfos.resize(num_glyphs);
            }

            hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer.get(), nullptr);
            hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer.get(), nullptr);

            // Check if all glyphs are valid.
            for (unsigned i=0; i<num_glyphs; ++i)
            {
                // if we have a valid codepoint, save rendering info.
                if (glyphs[i].codepoint)
                {
                    if (!glyphinfos[i].glyph.codepoint)
                    {
                        ++valid_glyphs;
                    }
                    glyphinfos[i] = { face, glyphs[i], positions[i] };
                }
            }
            if (valid_glyphs < num_glyphs && (pos < num_faces))
            {
                //Try next font in fontset
                continue;
            }

            double max_glyph_height = 0;
            for (unsigned i=0; i<num_glyphs; ++i)
            {
                auto& gpos = positions[i];
                auto& glyph = glyphs[i];
                face_ptr theface = face;
                if (glyphinfos[i].glyph.codepoint)
                {
                    gpos = glyphinfos[i].position;
                    glyph = glyphinfos[i].glyph;
                    theface = glyphinfos[i].face;
                }
                unsigned char_index = glyph.cluster;
                glyph_info g(glyph.codepoint,char_index,text_item.format_);
                if (theface->glyph_dimensions(g))
                {
                    g.face = theface;
                    g.scale_multiplier = size / theface->get_face()->units_per_EM;
                    //Overwrite default advance with better value provided by HarfBuzz
                    g.unscaled_advance = gpos.x_advance;
                    g.offset.set(gpos.x_offset * g.scale_multiplier, gpos.y_offset * g.scale_multiplier);
                    double tmp_height = g.height();
                    if (tmp_height > max_glyph_height) max_glyph_height = tmp_height;
                    width_map[char_index] += g.advance();
                    line.add_glyph(std::move(g), scale_factor);
                }
            }
            line.update_max_char_height(max_glyph_height);
            break; //When we reach this point the current font had all glyphs.
        }
    }
}
};
} // namespace mapnik

#endif // MAPNIK_HARFBUZZ_SHAPER_HPP
