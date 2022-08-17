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
// mapnik
#include <mapnik/text/face.hpp>
#include <mapnik/debug.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>

extern "C" {
#include FT_GLYPH_H
#include FT_TRUETYPE_TABLES_H
}

MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

font_face::font_face(FT_Face face)
    : face_(face)
    , color_font_(init_color_font())
{}

bool font_face::init_color_font()
{
    static const uint32_t tag = FT_MAKE_TAG('C', 'B', 'D', 'T');
    unsigned long length = 0;
    FT_Load_Sfnt_Table(face_, tag, 0, nullptr, &length);
    return length > 0;
}

bool font_face::set_character_sizes(double size)
{
    return (FT_Set_Char_Size(face_, 0, static_cast<FT_F26Dot6>(size * (1 << 6)), 0, 0) == 0);
}

bool font_face::set_unscaled_character_sizes()
{
    FT_F26Dot6 char_height = face_->units_per_EM > 0 ? face_->units_per_EM : 2048.0;
    return (FT_Set_Char_Size(face_, 0, char_height, 0, 0) == 0);
}

bool font_face::glyph_dimensions(glyph_info& glyph) const
{
    FT_Vector pen;
    pen.x = 0;
    pen.y = 0;
    if (color_font_)
        FT_Select_Size(face_, 0);
    FT_Set_Transform(face_, 0, &pen);
    FT_Int32 load_flags = FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING;
    if (color_font_)
        load_flags |= FT_LOAD_COLOR;
    if (FT_Load_Glyph(face_, glyph.glyph_index, load_flags))
    {
        MAPNIK_LOG_ERROR(font_face) << "FT_Load_Glyph failed :( index=" << glyph.glyph_index << " " << load_flags << " "
                                    << face_->family_name << " " << face_->style_name;
        return false;
    }
    FT_Glyph image;
    if (FT_Get_Glyph(face_->glyph, &image))
    {
        MAPNIK_LOG_ERROR(font_face) << "FT_Get_Glyph failed";
        return false;
    }
    FT_BBox glyph_bbox;
    FT_Glyph_Get_CBox(image, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);
    FT_Done_Glyph(image);
    glyph.unscaled_ymin = glyph_bbox.yMin;
    glyph.unscaled_ymax = glyph_bbox.yMax;
    glyph.unscaled_advance = face_->glyph->advance.x;
    glyph.unscaled_line_height = face_->size->metrics.height;

    if (color_font_)
    {
        double scale_multiplier = 2048.0 / (face_->size->metrics.height);
        glyph.unscaled_ymin *= scale_multiplier;
        glyph.unscaled_ymax *= scale_multiplier;
        glyph.unscaled_advance *= scale_multiplier;
    }

    return true;
}

font_face::~font_face()
{
    MAPNIK_LOG_DEBUG(font_face) << "font_face: Clean up face \"" << family_name() << " " << style_name() << "\"";

    FT_Done_Face(face_);
}

/******************************************************************************/

void font_face_set::add(face_ptr face)
{
    faces_.push_back(face);
}

void font_face_set::set_character_sizes(double _size)
{
    for (face_ptr const& face : faces_)
    {
        face->set_character_sizes(_size);
    }
}

void font_face_set::set_unscaled_character_sizes()
{
    for (face_ptr const& face : faces_)
    {
        face->set_unscaled_character_sizes();
    }
}

/******************************************************************************/

void stroker::init(double radius)
{
    FT_Stroker_Set(s_,
                   static_cast<FT_Fixed>(radius * (1 << 6)),
                   FT_STROKER_LINECAP_ROUND,
                   FT_STROKER_LINEJOIN_ROUND,
                   0);
}

stroker::~stroker()
{
    MAPNIK_LOG_DEBUG(font_engine_freetype) << "stroker: Destroy stroker=" << s_;

    FT_Stroker_Done(s_);
}

} // namespace mapnik
