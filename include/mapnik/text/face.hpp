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
#ifndef MAPNIK_FACE_HPP
#define MAPNIK_FACE_HPP

//mapnik
#include <mapnik/config.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/util/noncopyable.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>

extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

#pragma GCC diagnostic pop

//stl
#include <memory>
#include <string>
#include <vector>

namespace mapnik
{

class MAPNIK_DECL font_face : util::noncopyable
{
public:
    font_face(FT_Face face);

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

    bool set_character_sizes(double size);
    bool set_unscaled_character_sizes();

    bool glyph_dimensions(glyph_info &glyph) const;

    ~font_face();

private:
    FT_Face face_;
};
using face_ptr = std::shared_ptr<font_face>;


class MAPNIK_DECL font_face_set : private util::noncopyable
{
public:
    using iterator = std::vector<face_ptr>::iterator;
    font_face_set(void) : faces_(){}

    void add(face_ptr face);
    void set_character_sizes(double size);
    void set_unscaled_character_sizes();

    std::size_t size() const { return faces_.size(); }
    iterator begin() { return faces_.begin(); }
    iterator end() { return faces_.end(); }
private:
    std::vector<face_ptr> faces_;
};
using face_set_ptr = std::unique_ptr<font_face_set>;


// FT_Stroker wrapper
class stroker : util::noncopyable
{
public:
    explicit stroker(FT_Stroker s)
        : s_(s) {}
    ~stroker();

    void init(double radius);
    FT_Stroker const& get() const { return s_; }
private:
    FT_Stroker s_;
};

} //ns mapnik

#endif // FACE_HPP
