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
// mapnik
#include <mapnik/text/face.hpp>
#include <mapnik/debug.hpp>

extern "C"
{
#include FT_GLYPH_H
}

namespace mapnik
{

font_face::font_face(FT_Face face)
    : face_(face) {}

bool font_face::set_character_sizes(double size)
{
    return (FT_Set_Char_Size(face_,0,(FT_F26Dot6)(size * (1<<6)),0,0) == 0);
}

bool font_face::set_unscaled_character_sizes()
{
    return (FT_Set_Char_Size(face_,0,face_->units_per_EM,0,0) == 0);
}

bool font_face::glyph_dimensions(glyph_info & glyph) const
{
    FT_Vector pen;
    pen.x = 0;
    pen.y = 0;
    FT_Set_Transform(face_, 0, &pen);

    if (FT_Load_Glyph(face_, glyph.glyph_index, FT_LOAD_NO_HINTING))
    {
        MAPNIK_LOG_ERROR(font_face) << "FT_Load_Glyph failed";
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
    return true;
}

font_face::~font_face()
{
    MAPNIK_LOG_DEBUG(font_face) <<
        "font_face: Clean up face \"" << family_name() <<
        " " << style_name() << "\"";

    FT_Done_Face(face_);
}

/******************************************************************************/

void font_face_set::add(face_ptr face)
{
    faces_.push_back(face);
}

void font_face_set::set_character_sizes(double size)
{
    for (face_ptr const& face : faces_)
    {
        face->set_character_sizes(size);
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
    FT_Stroker_Set(s_, (FT_Fixed) (radius * (1<<6)),
                   FT_STROKER_LINECAP_ROUND,
                   FT_STROKER_LINEJOIN_ROUND,
                   0);
}

stroker::~stroker()
{
    MAPNIK_LOG_DEBUG(font_engine_freetype) << "stroker: Destroy stroker=" << s_;

    FT_Stroker_Done(s_);
}

}//ns mapnik
