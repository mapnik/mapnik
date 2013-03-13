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


#ifndef MAPNIK_FONT_UTIL_HPP
#define MAPNIK_FONT_UTIL_HPP

// mapnik
#include <mapnik/noncopyable.hpp>

#include <string>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H
}

namespace mapnik
{
struct char_properties;

struct glyph_t : mapnik::noncopyable
{
    FT_Glyph image;
    char_properties *properties;
    glyph_t(FT_Glyph image_, char_properties *properties_)
        : image(image_),
        properties(properties_) {}
    ~glyph_t()
    {
        FT_Done_Glyph(image);
    }
};


// FT_Stroker wrapper
class stroker : mapnik::noncopyable
{
public:
    explicit stroker(FT_Stroker s)
        : s_(s) {}

    void init(double radius)
    {
        FT_Stroker_Set(s_, (FT_Fixed) (radius * (1<<6)),
                       FT_STROKER_LINECAP_ROUND,
                       FT_STROKER_LINEJOIN_ROUND,
                       0);
    }

    FT_Stroker const& get() const
    {
        return s_;
    }

    ~stroker()
    {
        FT_Stroker_Done(s_);
    }
private:
    FT_Stroker s_;
};


class font_face : mapnik::noncopyable
{
public:
    font_face(FT_Face face)
        : face_(face) {}

    std::string family_name() const
    {
        return std::string(face_->family_name);
    }

    std::string style_name() const
    {
        return std::string(face_->style_name);
    }

    FT_GlyphSlot glyph() const
    {
        return face_->glyph;
    }

    FT_Face get_face() const
    {
        return face_;
    }

    unsigned get_char(unsigned c) const
    {
        return FT_Get_Char_Index(face_, c);
    }

    bool set_pixel_sizes(unsigned size)
    {
        if (! FT_Set_Pixel_Sizes( face_, 0, size ))
            return true;
        return false;
    }

    bool set_character_sizes(double size)
    {
        if ( !FT_Set_Char_Size(face_,0,(FT_F26Dot6)(size * (1<<6)),0,0))
            return true;
        return false;
    }

    ~font_face()
    {
        FT_Done_Face(face_);
    }

private:
    FT_Face face_;
};

}


#endif // MAPNIK_FONT_UTIL_HPP
