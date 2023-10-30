/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

// mapnik
#include <mapnik/text/evaluated_format_properties_ptr.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/util/noncopyable.hpp>

#include <memory>
#include <cmath>

namespace mapnik {

class font_face;
using face_ptr = std::shared_ptr<font_face>;

struct glyph_info : util::noncopyable
{
    glyph_info(unsigned g_index, unsigned c_index, evaluated_format_properties_ptr const& f)
        : glyph_index(g_index)
        , char_index(c_index)
        , format(f)
        , face(nullptr)
        , unscaled_ymin(0.0)
        , unscaled_ymax(0.0)
        , unscaled_advance(0.0)
        , unscaled_line_height(0.0)
        , scale_multiplier(1.0)
        , offset()
    {}
    glyph_info(glyph_info&& rhs)
        : glyph_index(std::move(rhs.glyph_index))
        , char_index(std::move(rhs.char_index))
        , format(rhs.format)
        , // take ref
        face(std::move(rhs.face))
        , // shared_ptr move just ref counts, right?
        unscaled_ymin(std::move(rhs.unscaled_ymin))
        , unscaled_ymax(std::move(rhs.unscaled_ymax))
        , unscaled_advance(std::move(rhs.unscaled_advance))
        , unscaled_line_height(std::move(rhs.unscaled_line_height))
        , scale_multiplier(std::move(rhs.scale_multiplier))
        , offset(std::move(rhs.offset))
    {}

    unsigned glyph_index;
    // Position in the string of all characters i.e. before itemizing
    unsigned char_index;
    evaluated_format_properties_ptr const& format;
    face_ptr face;
    double unscaled_ymin;
    double unscaled_ymax;
    double unscaled_advance;
    // Line height returned by freetype, includes normal font
    // line spacing, but not additional user defined spacing
    double unscaled_line_height;
    double scale_multiplier;
    pixel_position offset;

    double ymin() const { return unscaled_ymin * 64.0 * scale_multiplier; }
    double ymax() const { return unscaled_ymax * 64.0 * scale_multiplier; }
    double height() const { return ymax() - ymin(); };
    double advance() const { return unscaled_advance * scale_multiplier; };
    double line_height() const { return unscaled_line_height * scale_multiplier; };
};

} // namespace mapnik

#endif // GLYPH_INFO_HPP
