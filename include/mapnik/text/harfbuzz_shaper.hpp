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
#include <mapnik/text/itemizer.hpp>
#include <mapnik/safe_cast.hpp>
#include <mapnik/font_engine_freetype.hpp>

// stl
#include <cstdint>
#include <list>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

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
    // Glyph data emitted by HarfBuzz before applying the requested text size.
    struct shaped_glyph
    {
        face_ptr face;
        unsigned codepoint;
        unsigned cluster;
        int x_advance;
        int x_offset;
        int y_offset;
    };

    struct shaping_key
    {
        std::u16string text;
        unsigned start;
        unsigned end;
        bool rtl;
        int script;
        bool has_lang;
        std::string lang;
        std::string face_name;
        std::string fontset;
        std::string features;

        bool operator==(shaping_key const& other) const
        {
            return start == other.start && end == other.end && rtl == other.rtl && script == other.script &&
                   text == other.text && has_lang == other.has_lang && lang == other.lang &&
                   face_name == other.face_name && fontset == other.fontset && features == other.features;
        }
    };

    struct shaping_key_hash
    {
        std::size_t operator()(shaping_key const& k) const
        {
            std::size_t seed = std::hash<std::u16string>()(k.text);
            auto mix = [&seed](std::size_t v) {
                seed ^= v + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
            };
            mix(k.start);
            mix(k.end);
            mix(static_cast<std::size_t>(k.rtl));
            mix(static_cast<std::size_t>(k.script));
            mix(static_cast<std::size_t>(k.has_lang));
            mix(std::hash<std::string>()(k.lang));
            mix(std::hash<std::string>()(k.face_name));
            mix(std::hash<std::string>()(k.fontset));
            mix(std::hash<std::string>()(k.features));
            return seed;
        }
    };

    using shaping_cache = std::unordered_map<shaping_key, std::vector<shaped_glyph>, shaping_key_hash>;

    static void append_u32(std::string& key, std::uint32_t value)
    {
        key.append(reinterpret_cast<char const*>(&value), sizeof(value));
    }

    static std::string fontset_key(std::optional<font_set> const& fset)
    {
        std::string key;
        key.push_back(fset ? 1 : 0);
        if (!fset)
        {
            return key;
        }
        std::size_t size = 1;
        for (std::string const& name : fset->get_face_names())
        {
            size += sizeof(std::uint32_t) + name.size();
        }
        key.reserve(size);
        for (std::string const& name : fset->get_face_names())
        {
            append_u32(key, static_cast<std::uint32_t>(name.size()));
            key.append(name);
        }
        return key;
    }

    static std::string feature_key(font_feature_settings const& settings)
    {
        std::string key;
        key.reserve(settings.count() * sizeof(hb_feature_t));
        for (hb_feature_t const& feature : settings.features())
        {
            append_u32(key, feature.tag);
            append_u32(key, feature.value);
            append_u32(key, feature.start);
            append_u32(key, feature.end);
        }
        return key;
    }

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
            double size = text_item.format_->text_size * scale_factor;

            font_feature_settings const& ff_settings = text_item.format_->ff_settings;
            int ff_count = safe_cast<int>(ff_settings.count());

            double max_glyph_height = 0;
            auto emit_glyph = [&](shaped_glyph const& sg, bool ensure_sizes) {
                glyph_info g(sg.codepoint, sg.cluster, text_item.format_);
                g.face = sg.face;
                if (ensure_sizes)
                {
                    g.face->set_unscaled_character_sizes();
                }
                if (g.face->glyph_dimensions(g))
                {
                    g.scale_multiplier = g.face->get_face()->units_per_EM > 0
                                           ? (size / g.face->get_face()->units_per_EM)
                                           : (size / 2048.0);
                    // Overwrite default advance with better value provided by HarfBuzz
                    g.unscaled_advance = sg.x_advance;
                    g.offset.set(sg.x_offset * g.scale_multiplier, sg.y_offset * g.scale_multiplier);
                    double tmp_height = g.height();
                    if (g.face->is_color())
                    {
                        tmp_height = g.ymax();
                    }
                    if (tmp_height > max_glyph_height)
                        max_glyph_height = tmp_height;
                    width_map[sg.cluster] += g.advance();
                    line.add_glyph(std::move(g), scale_factor);
                }
            };

            shaping_cache& cache = font_manager.get_shaper_cache<shaping_cache>();
            shaping_key key{std::u16string(reinterpret_cast<char16_t const*>(text.getBuffer()),
                                           static_cast<std::size_t>(text.length())),
                            text_item.start,
                            text_item.end,
                            text_item.dir == UBIDI_RTL,
                            static_cast<int>(text_item.script),
                            lang.has_value(),
                            lang.value_or(std::string()),
                            text_item.format_->face_name,
                            fontset_key(text_item.format_->fontset),
                            feature_key(ff_settings)};
            auto cache_hit = cache.find(key);
            if (cache_hit != cache.end())
            {
                for (shaped_glyph const& sg : cache_hit->second)
                {
                    emit_glyph(sg, true);
                }
                line.update_max_char_height(max_glyph_height);
                continue;
            }
            constexpr std::size_t max_cache_entries = 65536;
            bool const do_cache = cache.size() < max_cache_entries;
            std::vector<shaped_glyph> shaped;

            face_set_ptr face_set = font_manager.get_face_set(text_item.format_->face_name, text_item.format_->fontset);
            face_set->set_unscaled_character_sizes();
            std::size_t num_faces = face_set->size();

            // rendering information for a single glyph
            struct glyph_face_info
            {
                face_ptr face;
                hb_glyph_info_t glyph;
                hb_glyph_position_t position;
            };

            // this table is filled with information for rendering each glyph, so that
            // several font faces can be used in a single text_item
            std::size_t pos = 0;
            std::vector<std::vector<glyph_face_info>> glyphinfos;

            glyphinfos.resize(text.length());
            for (auto const& face : *face_set)
            {
                ++pos;
                hb_buffer_clear_contents(buffer.get());
                hb_buffer_add_utf16(buffer.get(),
                                    detail::uchar_to_utf16(text.getBuffer()),
                                    text.length(),
                                    text_item.start,
                                    static_cast<int>(text_item.end - text_item.start));
                hb_buffer_set_direction(buffer.get(),
                                        (text_item.dir == UBIDI_RTL) ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);

                hb_font_t* font = face->get_harfbuzz_font();
                auto script = detail::_icu_script_to_script(text_item.script);
                hb_language_t hb_lang;
                if (lang)
                {
                    hb_lang = hb_language_from_string(lang->c_str(), -1);
                }
                else
                {
                    hb_lang = detail::script_to_language(script);
                    MAPNIK_LOG_DEBUG(harfbuzz_shaper)
                      << "RUN:[" << text_item.start << "," << text_item.end << "]"
                      << " LANGUAGE:" << ((hb_lang != nullptr) ? hb_language_to_string(hb_lang) : "unknown")
                      << " SCRIPT:" << script << "(" << text_item.script << ") "
                      << uscript_getShortName(text_item.script) << " FONT:" << face->family_name();
                }
                if (hb_lang != HB_LANGUAGE_INVALID)
                {
                    hb_buffer_set_language(buffer.get(), hb_lang); // set most common language for the run based script
                }
                hb_buffer_set_script(buffer.get(), script);

                // https://github.com/mapnik/test-data-visual/pull/25
#if HB_VERSION_MAJOR > 0
#if HB_VERSION_ATLEAST(1, 0, 5)
                hb_ft_font_set_load_flags(font, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING);
#endif
#endif
                hb_shape(font, buffer.get(), ff_settings.get_features(), ff_count);

                unsigned num_glyphs = hb_buffer_get_length(buffer.get());
                hb_glyph_info_t* glyphs = hb_buffer_get_glyph_infos(buffer.get(), &num_glyphs);
                hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer.get(), &num_glyphs);

                unsigned cluster = 0;
                std::vector<unsigned> clusters;
                std::vector<std::vector<glyph_face_info>> current_clusters;
                current_clusters.resize(text.length());

                for (unsigned i = 0; i < num_glyphs; ++i)
                {
                    if (i == 0)
                    {
                        cluster = glyphs[0].cluster;
                        clusters.push_back(cluster);
                    }
                    if (cluster != glyphs[i].cluster)
                    {
                        cluster = glyphs[i].cluster;
                        clusters.push_back(cluster);
                    }
                    if (glyphinfos.size() <= cluster)
                    {
                        glyphinfos.resize(cluster + 1);
                    }
                    current_clusters[cluster].push_back({face, glyphs[i], positions[i]});
                }
                for (unsigned cluster_id = 0; cluster_id < current_clusters.size(); ++cluster_id)
                {
                    auto const& cluster_glyphs = current_clusters[cluster_id];

                    if (cluster_glyphs.empty())
                        continue;
                    bool valid = true;
                    for (auto const& info : cluster_glyphs)
                    {
                        if (info.glyph.codepoint == 0)
                        {
                            valid = false;
                            break;
                        }
                    }
                    if (glyphinfos[cluster_id].empty())
                    {
                        if (valid || pos == num_faces)
                        {
                            glyphinfos[cluster_id] = cluster_glyphs;
                        }
                    }
                }
                bool all_set = true;
                for (auto c_id : clusters)
                {
                    auto const& c = glyphinfos[c_id];
                    if (c.empty() || c.front().glyph.codepoint == 0)
                    {
                        all_set = false;
                        break;
                    }
                }
                if (!all_set && (pos < num_faces))
                {
                    // Try next font in fontset
                    continue;
                }
                for (auto const& c_id : clusters)
                {
                    auto const& c = glyphinfos[c_id];
                    for (auto const& info : c)
                    {
                        shaped_glyph const sg{info.glyph.codepoint != 0 ? info.face : face,
                                              info.glyph.codepoint,
                                              info.glyph.cluster,
                                              info.position.x_advance,
                                              info.position.x_offset,
                                              info.position.y_offset};
                        emit_glyph(sg, false);
                        if (do_cache)
                        {
                            shaped.push_back(sg);
                        }
                    }
                }
                line.update_max_char_height(max_glyph_height);
                if (do_cache)
                {
                    cache.emplace(std::move(key), std::move(shaped));
                }
                break; // When we reach this point the current font had all glyphs.
            }
        }
    }
};
} // namespace mapnik

#endif // MAPNIK_HARFBUZZ_SHAPER_HPP
