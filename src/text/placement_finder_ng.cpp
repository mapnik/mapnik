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

//boost
#include <boost/make_shared.hpp>

namespace mapnik
{

placement_finder_ng::placement_finder_ng( Feature const& feature, DetectorType &detector, box2d<double> const& extent)
    : feature_(feature), detector_(detector), extent_(extent)
{
}

glyph_positions_ptr placement_finder_ng::find_point_placement(text_layout_ptr layout, double pos_x, double pos_y, double angle)
{
    glyph_positions_ptr glyphs = boost::make_shared<glyph_positions>(layout);
    glyphs->point_placement(pixel_position(pos_x, pos_y));
    //TODO: angle
    //TODO: Check for placement
    return glyphs;
}

glyph_positions::glyph_positions(text_layout_ptr layout)
    : base_point_(), point_(true), layout_(layout), current_(0)
{

}

void glyph_positions::point_placement(pixel_position base_point)
{
    base_point_ = base_point;
    point_ = true;
}

bool glyph_positions::next()
{
    return false;
#if 0
    if (current_ == -1)
    {
        current_ = 0;
        return (bool)layout_->size();
    }
    if (current_ >= layout_->size()) return false;
    glyph_info glyph = layout_->get_glyphs()[current_];
    current_position_.x += glyph.width + glyph.format->character_spacing;
    current_++;
    if (current_ >= layout_->size()) return false;
    return true;
#endif
}

void glyph_positions::rewind()
{
    current_ = -1;
    current_position_ = pixel_position(0, 0);
}

glyph_info const& glyph_positions::get_glyph() const
{
//    assert(layout_);
//    assert(current_ < layout_->size());
//    return layout_->get_glyphs()[current_];
}

pixel_position glyph_positions::get_position() const
{
    return current_position_;
}

double glyph_positions::get_angle() const
{
    return 0;
}

bool glyph_positions::is_constant_angle() const
{
    return point_;
}

const pixel_position &glyph_positions::get_base_point() const
{
    return base_point_;
}


}// ns mapnik
