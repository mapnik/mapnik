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
#ifndef MAPNIK_TEXT_SHAPING_HPP
#define MAPNIK_TEXT_SHAPING_HPP

//ICU
#include <unicode/unistr.h>
#include <unicode/uscript.h>
#include <harfbuzz/hb.h>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
}

namespace mapnik
{

/** This class is a thin wrapper around HarfBuzz. */
class text_shaping
{
public:
    text_shaping(FT_Face face);
    ~text_shaping();

    uint32_t process_text(UnicodeString const& text, unsigned start, unsigned end, bool rtl, UScriptCode script);
    hb_buffer_t *get_buffer() { return buffer_; }

protected:
    static void free_data(void *data);

    void load_font();

    hb_font_t *font_;
    hb_buffer_t *buffer_;
    FT_Face face_;
};
} //ns mapnik

#endif // TEXT_SHAPING_HPP
