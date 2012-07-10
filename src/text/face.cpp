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
#include <mapnik/text/face.hpp>
#include <mapnik/debug.hpp>

#include <boost/foreach.hpp>

extern "C"
{
#include FT_GLYPH_H
}

namespace mapnik
{

void font_face::glyph_dimensions(glyph_info &glyph)
{
    //TODO
    //Check if char is already in cache
//    std::map<glyph_index_t, glyph_info>::const_iterator itr;
//    itr = dimension_cache_.find(glyph.glyph_index);
//    if (itr != dimension_cache_.end()) {
//        glyph = itr->second;
//        return;
//    }

    FT_Matrix matrix;
    FT_Vector pen;

    pen.x = 0;
    pen.y = 0;

    FT_BBox glyph_bbox;
    FT_Glyph image;

    matrix.xx = (FT_Fixed)( 1 * 0x10000L );
    matrix.xy = (FT_Fixed)( 0 * 0x10000L );
    matrix.yx = (FT_Fixed)( 0 * 0x10000L );
    matrix.yy = (FT_Fixed)( 1 * 0x10000L );

    FT_Set_Transform(face_, &matrix, &pen); //TODO: Matrix is always only set to the identity matrix. This seems to be useless.

    if (FT_Load_Glyph (face_, glyph.glyph_index, FT_LOAD_NO_HINTING)) return;
    if (FT_Get_Glyph(face_->glyph, &image)) return;
    FT_Glyph_Get_CBox(image, ft_glyph_bbox_pixels, &glyph_bbox);
    FT_Done_Glyph(image);

    glyph.ymin = glyph_bbox.yMin; //TODO: Which data format? 26.6, integer?
    glyph.ymax = glyph_bbox.yMax; //TODO: Which data format? 26.6, integer?
    glyph.line_height = face_->size->metrics.height/64.0;

//TODO:    dimension_cache_.insert(std::pair<unsigned, char_info>(c, dim));
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

void font_face_set::set_character_sizes(float size)
{
    BOOST_FOREACH ( face_ptr const& face, faces_)
    {
        face->set_character_sizes(size);
    }
}

}//ns mapnik
