/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/text/clusters_container.hpp>
#include <mapnik/text/font_coverage.hpp>
#include <mapnik/text/itemizer.hpp>
#include <mapnik/safe_cast.hpp>
#include <mapnik/font_engine_freetype.hpp>

// stl
#include <cassert>
#include <algorithm>
#include <list>
#include <limits>
#include <optional>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <unicode/uvernum.h>
#include <unicode/uscript.h>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace detail {

static inline hb_script_t _icu_script_to_script(UScriptCode script)
{
    if (script == USCRIPT_INVALID_CODE)
        return HB_SCRIPT_INVALID;
    return hb_script_from_string(uscript_getShortName(script), -1);
}

static inline uint16_t const* uchar_to_utf16(UChar const* src)
{
    static_assert(sizeof(UChar) == sizeof(uint16_t), "UChar is eq size to uint16_t");
#if defined(_MSC_VER) || (U_ICU_VERSION_MAJOR_NUM >= 59)
    // ^^ http://site.icu-project.org/download/59#TOC-ICU4C-char16_t1
    return reinterpret_cast<uint16_t const*>(src);
#else
    return src;
#endif
}

static hb_language_t script_to_language(hb_script_t script)
{
    switch (script)
    {
        // Unicode 1.1
        case HB_SCRIPT_ARABIC:
            return hb_language_from_string("ar", -1);
            break;
        case HB_SCRIPT_ARMENIAN:
            return hb_language_from_string("hy", -1);
            break;
        case HB_SCRIPT_BENGALI:
            return hb_language_from_string("bn", -1);
            break;
        case HB_SCRIPT_CANADIAN_ABORIGINAL:
            return hb_language_from_string("iu", -1);
            break;
        case HB_SCRIPT_CHEROKEE:
            return hb_language_from_string("chr", -1);
            break;
        case HB_SCRIPT_COPTIC:
            return hb_language_from_string("cop", -1);
            break;
        case HB_SCRIPT_CYRILLIC:
            return hb_language_from_string("ru", -1);
            break;
        case HB_SCRIPT_DEVANAGARI:
            return hb_language_from_string("hi", -1);
            break;
        case HB_SCRIPT_GEORGIAN:
            return hb_language_from_string("ka", -1);
            break;
        case HB_SCRIPT_GREEK:
            return hb_language_from_string("el", -1);
            break;
        case HB_SCRIPT_GUJARATI:
            return hb_language_from_string("gu", -1);
            break;
        case HB_SCRIPT_GURMUKHI:
            return hb_language_from_string("pa", -1);
            break;
        case HB_SCRIPT_HANGUL:
            return hb_language_from_string("ko", -1);
            break;
        case HB_SCRIPT_HAN:
            return hb_language_from_string("zh-hans", -1);
            break;
        case HB_SCRIPT_HEBREW:
            return hb_language_from_string("he", -1);
            break;
        case HB_SCRIPT_HIRAGANA:
            return hb_language_from_string("ja", -1);
            break;
        case HB_SCRIPT_KANNADA:
            return hb_language_from_string("kn", -1);
            break;
        case HB_SCRIPT_KATAKANA:
            return hb_language_from_string("ja", -1);
            break;
        case HB_SCRIPT_LAO:
            return hb_language_from_string("lo", -1);
            break;
        case HB_SCRIPT_LATIN:
            return hb_language_from_string("en", -1);
            break;
        case HB_SCRIPT_MALAYALAM:
            return hb_language_from_string("ml", -1);
            break;
        case HB_SCRIPT_MONGOLIAN:
            return hb_language_from_string("mn", -1);
            break;
        case HB_SCRIPT_ORIYA:
            return hb_language_from_string("or", -1);
            break;
        case HB_SCRIPT_SYRIAC:
            return hb_language_from_string("syr", -1);
            break;
        case HB_SCRIPT_TAMIL:
            return hb_language_from_string("ta", -1);
            break;
        case HB_SCRIPT_TELUGU:
            return hb_language_from_string("te", -1);
            break;
        case HB_SCRIPT_THAI:
            return hb_language_from_string("th", -1);
            break;

        // Unicode 2.0
        case HB_SCRIPT_TIBETAN:
            return hb_language_from_string("bo", -1);
            break;

        // Unicode 3.0
        case HB_SCRIPT_ETHIOPIC:
            return hb_language_from_string("am", -1);
            break;
        case HB_SCRIPT_KHMER:
            return hb_language_from_string("km", -1);
            break;
        case HB_SCRIPT_MYANMAR:
            return hb_language_from_string("my", -1);
            break;
        case HB_SCRIPT_SINHALA:
            return hb_language_from_string("si", -1);
            break;
        case HB_SCRIPT_THAANA:
            return hb_language_from_string("dv", -1);
            break;

        // Unicode 3.2
        case HB_SCRIPT_BUHID:
            return hb_language_from_string("bku", -1);
            break;
        case HB_SCRIPT_HANUNOO:
            return hb_language_from_string("hnn", -1);
            break;
        case HB_SCRIPT_TAGALOG:
            return hb_language_from_string("tl", -1);
            break;
        case HB_SCRIPT_TAGBANWA:
            return hb_language_from_string("tbw", -1);
            break;

        // Unicode 4.0
        case HB_SCRIPT_UGARITIC:
            return hb_language_from_string("uga", -1);
            break;

        // Unicode 4.1
        case HB_SCRIPT_BUGINESE:
            return hb_language_from_string("bug", -1);
            break;
        case HB_SCRIPT_OLD_PERSIAN:
            return hb_language_from_string("peo", -1);
            break;
        case HB_SCRIPT_SYLOTI_NAGRI:
            return hb_language_from_string("syl", -1);
            break;

        // Unicode 5.0
        case HB_SCRIPT_NKO:
            return hb_language_from_string("nko", -1);
            break;

        // no representative language exists
        default:
            return HB_LANGUAGE_INVALID;
            break;
    }
}

} // namespace detail

struct harfbuzz_shaper
{
    static void shape_text(text_line& line,
                           text_itemizer& itemizer,
                           std::map<unsigned, double>& width_map,
                           face_manager_freetype& font_manager,
                           double scale_factor,
                           std::optional<std::string> lang = std::optional<std::string>(std::nullopt))
    {
        unsigned start = line.first_char();
        unsigned end = line.last_char();
        std::size_t length = end - start;
        if (!length)
            return;

        std::list<text_item> const& list = itemizer.itemize(start, end);

        line.reserve(length);

        auto hb_buffer_deleter = [](hb_buffer_t* buffer) {
            hb_buffer_destroy(buffer);
        };
        std::unique_ptr<hb_buffer_t, decltype(hb_buffer_deleter)> const buffer(hb_buffer_create(), hb_buffer_deleter);
        hb_buffer_pre_allocate(buffer.get(), safe_cast<int>(length));
        mapnik::value_unicode_string const& text = itemizer.text();
        for (auto const& text_item : list)
        {
            face_set_ptr face_set = font_manager.get_face_set(text_item.format_->face_name, text_item.format_->fontset);
            double size = text_item.format_->text_size * scale_factor;
            face_set->set_unscaled_character_sizes();
            std::size_t num_faces = face_set->size();

            font_feature_settings const& ff_settings = text_item.format_->ff_settings;
            int ff_count = safe_cast<int>(ff_settings.count());

            auto script = detail::_icu_script_to_script(text_item.script);
            bool rtl = text_item.dir == UBIDI_RTL;
            hb_direction_t direction = rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;
            hb_language_t hb_lang;
            bool lang_from_script = false;
            if (lang)
            {
                hb_lang = hb_language_from_string(lang->c_str(), -1);
            }
            else
            {
                hb_lang = detail::script_to_language(script);
                lang_from_script = true;
            }

            // this table is filled with information for rendering each glyph cluster, so that
            // several font faces can be used in a single text_item
            std::size_t pos = 0;
            struct glyph_face_info
            {
                face_ptr face;
                hb_glyph_info_t glyph;
                hb_glyph_position_t position;
            };
            using glyph_cluster = std::vector<glyph_face_info>;
            detail::font_coverage coverage(text_item.start, text_item.end);
            std::vector<glyph_cluster> glyphinfos;
            glyphinfos.reserve(length);
            for (auto const& face : *face_set)
            {
                if (lang_from_script)
                {
                    MAPNIK_LOG_DEBUG(harfbuzz_shaper)
                      << "RUN:[" << text_item.start << "," << text_item.end << "]"
                      << " LANGUAGE:" << ((hb_lang != nullptr) ? hb_language_to_string(hb_lang) : "unknown")
                      << " SCRIPT:" << script << "(" << text_item.script << ") "
                      << uscript_getShortName(text_item.script) << " FONT:" << face->family_name();
                }

                ++pos;
                bool last_face = pos == num_faces;
                hb_font_t* font(hb_ft_font_create(face->get_face(), nullptr));

                // https://github.com/mapnik/test-data-visual/pull/25
#if HB_VERSION_MAJOR > 0
#if HB_VERSION_ATLEAST(1, 0, 5)
                hb_ft_font_set_load_flags(font, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);
#endif
#endif

                struct shaped_cluster
                {
                    unsigned start;
                    unsigned end;
                    bool covered;
                    glyph_cluster glyphs;
                };
                constexpr unsigned open_cluster_end = std::numeric_limits<unsigned>::max();
                detail::clusters_container<shaped_cluster> clusters;

                while (coverage.has_current_uncovered())
                {
                    auto const range = coverage.pop_current_uncovered_front();

                    hb_buffer_clear_contents(buffer.get());
                    hb_buffer_add_utf16(buffer.get(),
                                        detail::uchar_to_utf16(text.getBuffer()),
                                        text.length(),
                                        range.start,
                                        static_cast<int>(range.end - range.start));
                    hb_buffer_set_direction(buffer.get(), direction);

                    if (hb_lang != HB_LANGUAGE_INVALID)
                    {
                        hb_buffer_set_language(buffer.get(),
                                               hb_lang); // set most common language for the run based script
                    }
                    hb_buffer_set_script(buffer.get(), script);

                    hb_shape(font, buffer.get(), ff_settings.get_features(), ff_count);

                    unsigned num_glyphs = hb_buffer_get_length(buffer.get());
                    hb_glyph_info_t* glyphs = hb_buffer_get_glyph_infos(buffer.get(), &num_glyphs);
                    hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer.get(), &num_glyphs);
                    if (!num_glyphs)
                    {
                        coverage.push_uncovered(range.start, range.end);
                        continue;
                    }

                    clusters.reset(num_glyphs);
                    assert(num_glyphs < open_cluster_end);

                    // The fallback bookkeeping below relies on HarfBuzz cluster ids staying
                    // tied to the original UTF-16 indices seeded by hb_buffer_add_utf16(...).
                    // Distinct cluster values therefore identify the first code unit of each
                    // shaped cluster within the full text_item range.
                    //
                    // We also assume HarfBuzz emits all glyphs for one cluster contiguously
                    // and that cluster ids are monotonic in output order.
                    //
                    // LTR: glyph order matches text order, so cluster ids increase.
                    // RTL: glyph order is reversed, but cluster ids still refer to the same
                    // logical UTF-16 cells, so they decrease in output order.
                    //
                    // In both cases each distinct HarfBuzz cluster id marks the first UTF-16
                    // code unit of a shaped cluster. For example, ids 0 -> 3 -> 6 describe
                    // logical ranges [0,3), [3,6), and [6,range.end), regardless of whether
                    // HarfBuzz reported them in LTR order (0, 3, 6) or RTL order (6, 3, 0).
                    //
                    if (!rtl)
                    {
                        //   utf16 cells        | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
                        //   hb cluster ids     | 0 | 3 | 6 |
                        //   buffer state       | [0,open) |
                        //   see next id = 3      -> close back() with end = 3
                        //   buffer state       | [0,3) | [3,open) |
                        //   see next id = 6      -> close back() with end = 6
                        //   buffer state       | [0,3) | [3,6) | [6,open) |
                        //   loop ends           -> close the remaining open cluster with range.end
                        //   final              | [0,3) | [3,6) | [6,8) |
                        for (unsigned i = 0; i < num_glyphs; ++i)
                        {
                            auto const cluster_start = glyphs[i].cluster;
                            bool const new_cluster = clusters.empty() || clusters.back().start != cluster_start;
                            if (new_cluster)
                            {
                                if (!clusters.empty())
                                    clusters.back().end = cluster_start;
                                shaped_cluster cluster = {cluster_start, open_cluster_end, true, {}};
                                clusters.push_back(std::move(cluster));
                            }
                            auto& cluster = clusters.back();
                            cluster.covered = cluster.covered && glyphs[i].codepoint != 0;
                            cluster.glyphs.push_back({face, glyphs[i], positions[i]});
                        }
                        if (!clusters.empty())
                            clusters.back().end = range.end;
                    }
                    else
                    {
                        //   utf16 cells        | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
                        //   hb cluster ids     | 6 | 3 | 0 |
                        //   buffer state       | [6,8) |
                        //   see next id = 3      -> new earlier cluster ends at front().start = 6
                        //   push_front([3,6))
                        //   buffer state       | [3,6) | [6,8) |
                        //   see next id = 0      -> new earlier cluster ends at front().start = 3
                        //   push_front([0,3))
                        //   buffer state       | [0,3) | [3,6) | [6,8) |
                        //   loop ends           -> no cluster remains open
                        //   final              | [0,3) | [3,6) | [6,8) |
                        for (unsigned i = 0; i < num_glyphs; ++i)
                        {
                            auto const cluster_start = glyphs[i].cluster;
                            bool const new_cluster = clusters.empty() || clusters.front().start != cluster_start;
                            if (new_cluster)
                            {
                                unsigned cluster_end = range.end;
                                if (!clusters.empty())
                                    cluster_end = clusters.front().start;
                                shaped_cluster cluster = {cluster_start, cluster_end, true, {}};
                                clusters.push_front(std::move(cluster));
                            }
                            auto& cluster = clusters.front();
                            cluster.covered = cluster.covered && glyphs[i].codepoint != 0;
                            cluster.glyphs.push_back({face, glyphs[i], positions[i]});
                        }
                    }
                    coverage.reserve(clusters.size());
                    for (auto& cluster : clusters)
                    {
                        if (!cluster.covered && !last_face)
                        {
                            coverage.push_uncovered(cluster.start, cluster.end);
                            continue;
                        }

                        glyphinfos.push_back(std::move(cluster.glyphs));
                        coverage.cover(cluster.start, cluster.end, glyphinfos.size() - 1);
                    }
                }
                hb_font_destroy(font);
                if (!coverage.fully_covered() && !last_face)
                {
                    coverage.advance_uncovered_ranges();
                    // Try next font in fontset
                    continue;
                }

                double max_glyph_height = 0;
                for (auto glyphinfo_itr = coverage.covering_begin(rtl); glyphinfo_itr != coverage.covering_end(rtl);
                     ++glyphinfo_itr)
                {
                    auto const glyphinfo_index = *glyphinfo_itr;
                    auto const& cluster = glyphinfos[glyphinfo_index];
                    for (auto const& info : cluster)
                    {
                        auto const& gpos = info.position;
                        auto const& glyph = info.glyph;
                        unsigned char_index = glyph.cluster;
                        glyph_info g(glyph.codepoint, char_index, text_item.format_);
                        g.face = info.face;
                        if (g.face->glyph_dimensions(g))
                        {
                            g.scale_multiplier = g.face->get_face()->units_per_EM > 0
                                                   ? (size / g.face->get_face()->units_per_EM)
                                                   : (size / 2048.0);
                            // Overwrite default advance with better value provided by HarfBuzz
                            g.unscaled_advance = gpos.x_advance;
                            g.offset.set(gpos.x_offset * g.scale_multiplier, gpos.y_offset * g.scale_multiplier);
                            double tmp_height = g.height();
                            if (g.face->is_color())
                            {
                                tmp_height = g.ymax();
                            }
                            if (tmp_height > max_glyph_height)
                                max_glyph_height = tmp_height;
                            width_map[char_index] += g.advance();
                            line.add_glyph(std::move(g), scale_factor);
                        }
                    }
                }
                line.update_max_char_height(max_glyph_height);
                break; // When we reach this point the current font had all glyphs.
            }
        }
    }
};
} // namespace mapnik

#endif // MAPNIK_HARFBUZZ_SHAPER_HPP
