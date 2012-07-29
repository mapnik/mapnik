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
//mapnik
#include <mapnik/text/placement_finder_ng.hpp>
#include <mapnik/text/layout.hpp>
#include <mapnik/text_properties.hpp>
#include <mapnik/debug.hpp>

//boost
#include <boost/make_shared.hpp>

namespace mapnik
{

placement_finder_ng::placement_finder_ng(Feature const& feature, DetectorType &detector, box2d<double> const& extent, text_placement_info_ptr placement_info, face_manager_freetype &font_manager, double scale_factor)
    : feature_(feature), detector_(detector), extent_(extent), layout_(font_manager), info_(placement_info), valid_(true), scale_factor_(scale_factor)
{
}

bool placement_finder_ng::next_position()
{
    if (!valid_)
    {
        MAPNIK_LOG_WARN(placement_finder_ng) << "next_position() called while last call already returned false!\n";
        return false;
    }
    if (!info_->next())
    {
        valid_ = false;
        return false;
    }

    info_->properties.process(layout_, feature_);
    layout_.layout(info_->properties.wrap_width, info_->properties.text_ratio);

    if (info_->properties.orientation)
    {
        angle_ = boost::apply_visitor(
            evaluate<Feature, value_type>(feature_),
            *(info_->properties.orientation)).to_double() * M_PI / 180.0;
    } else {
        angle_ = 0.0;
    }
    cosa_ = std::cos(angle_);
    sina_ = std::sin(angle_);

    init_alignment();
    return true;
}


void placement_finder_ng::init_alignment()
{
    text_symbolizer_properties const& p = info_->properties;
    valign_ = p.valign;
    if (valign_ == V_AUTO)
    {
        if (p.displacement.y > 0.0)
        {
            valign_ = V_BOTTOM;
        } else if (p.displacement.y < 0.0)
        {
            valign_ = V_TOP;
        } else
        {
            valign_ = V_MIDDLE;
        }
    }

    halign_ = p.halign;
    if (halign_ == H_AUTO)
    {
        if (p.displacement.x > 0.0)
        {
            halign_ = H_RIGHT;
        } else if (p.displacement.x < 0.0)
        {
            halign_ = H_LEFT;
        } else
        {
            halign_ = H_MIDDLE;
        }
    }

    jalign_ = p.jalign;
    if (jalign_ == J_AUTO)
    {
        if (p.displacement.x > 0.0)
        {
            jalign_ = J_LEFT;
        } else if (p.displacement.x < 0.0)
        {
            jalign_ = J_RIGHT;
        } else {
            jalign_ = J_MIDDLE;
        }
    }
}


pixel_position placement_finder_ng::alignment_offset() const
{
    pixel_position result(0,0);
    // if needed, adjust for desired vertical alignment
    if (valign_ == V_TOP)
    {
        result.y = -0.5 * layout_.height();  // move center up by 1/2 the total height
    } else if (valign_ == V_BOTTOM)
    {
        result.y = 0.5 * layout_.height();  // move center down by the 1/2 the total height
    }

    // set horizontal position to middle of text
    if (halign_ == H_LEFT)
    {
        result.x = -0.5 * layout_.width();  // move center left by 1/2 the string width
    } else if (halign_ == H_RIGHT)
    {
        result.x = 0.5 * layout_.width();  // move center right by 1/2 the string width
    }
    return result;
}


glyph_positions_ptr placement_finder_ng::find_point_placement(pixel_position pos)
{
    glyph_positions_ptr glyphs = boost::make_shared<glyph_positions>();
    if (!layout_.size()) return glyphs; /* No data. Don't return NULL pointer, which would mean
     that not enough space was available. */

    //TODO: Verify enough space is available. For point placement the bounding box is enough!

    glyphs->set_base_point(pos + scale_factor_ * info_->properties.displacement + alignment_offset());

    /* IMPORTANT NOTE:
       x and y are relative to the center of the text
       coordinate system:
       x: grows from left to right
       y: grows from bottom to top (opposite of normal computer graphics)
    */
    double x, y;

    // set for upper left corner of text envelope for the first line, top left of first character
    y = layout_.height() / 2.0;

    text_layout::const_iterator line_itr = layout_.begin(), line_end = layout_.end();
    for (; line_itr != line_end; line_itr++)
    {
        y -= (*line_itr)->height(); //Automatically handles first line differently
        // reset to begining of line position
        if (jalign_ == J_LEFT)
            x = -(layout_.width() / 2.0);
        else if (jalign_ == J_RIGHT)
            x = (layout_.width() / 2.0) - (*line_itr)->width();
        else
            x = -((*line_itr)->width() / 2.0);

        text_line::const_iterator glyph_itr = (*line_itr)->begin(), glyph_end = (*line_itr)->end();
        for (; glyph_itr != glyph_end; glyph_itr++)
        {
            glyphs->push_back(*glyph_itr, pixel_position(x, y), angle_); //TODO: Store cosa, sina instead
            x += glyph_itr->width + glyph_itr->format->character_spacing;
        }
    }
    return glyphs;
}


/*********************************************************************************************/


glyph_positions::glyph_positions()
    : base_point_(), const_angle_(true)
{

}

glyph_positions::const_iterator glyph_positions::begin() const
{
    return data_.begin();
}

glyph_positions::const_iterator glyph_positions::end() const
{
    return data_.end();
}

void glyph_positions::push_back(const glyph_info &glyph, pixel_position offset, double angle)
{
    if (data_.empty())
    {
        angle_ = angle;
    } else
    {
        if (angle != angle_) const_angle_ = false;
    }
    data_.push_back(glyph_position(glyph, offset, angle));
}


bool glyph_positions::is_constant_angle() const
{
    return const_angle_;
}

double glyph_positions::get_angle() const
{
    return angle_;
}

pixel_position const& glyph_positions::get_base_point() const
{
    return base_point_;
}

void glyph_positions::set_base_point(pixel_position base_point)
{
    base_point_ = base_point;
}


}// ns mapnik
