/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/glyph_info.hpp>

// stl
#include <vector>

namespace mapnik {

glyph_positions::glyph_positions()
    : data_()
    , base_point_()
    , marker_info_()
    , marker_pos_()
    , bbox_()
{}

glyph_positions::const_iterator glyph_positions::begin() const
{
    return data_.begin();
}

glyph_positions::const_iterator glyph_positions::end() const
{
    return data_.end();
}

void glyph_positions::emplace_back(glyph_info const& glyph, pixel_position offset, rotation const& rot)
{
    data_.emplace_back(glyph, offset, rot);
}

void glyph_positions::reserve(unsigned count)
{
    data_.reserve(count);
}

pixel_position const& glyph_positions::get_base_point() const
{
    return base_point_;
}

void glyph_positions::set_base_point(pixel_position const& base_point)
{
    base_point_ = base_point;
}

void glyph_positions::set_marker(marker_info_ptr mark, pixel_position const& marker_pos)
{
    marker_info_ = mark;
    marker_pos_ = marker_pos;
}

marker_info_ptr const& glyph_positions::get_marker() const
{
    return marker_info_;
}

pixel_position const& glyph_positions::marker_pos() const
{
    return marker_pos_;
}

} // namespace mapnik
