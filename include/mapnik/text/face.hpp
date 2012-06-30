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
#ifndef MAPNIK_FACE_HPP
#define MAPNIK_FACE_HPP

//mapnik
#include <mapnik/text/glyph_info.hpp>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
}

//boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

//stl
#include <map>
#include <string>

namespace mapnik
{

class font_face : boost::noncopyable
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

    bool set_character_sizes(float size)
    {
        if ( !FT_Set_Char_Size(face_,0,(FT_F26Dot6)(size * (1<<6)),0,0))
            return true;
        return false;
    }

    void glyph_dimensions(glyph_info &glyph);

    ~font_face();

private:
    FT_Face face_;
    std::map<glyph_index_t, glyph_info> dimension_cache_;
};

} //ns mapnik

#endif // FACE_HPP
