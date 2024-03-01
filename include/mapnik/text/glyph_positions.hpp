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

#ifndef MAPNIK_TEXT_GLYPH_POSITIONS_HPP
#define MAPNIK_TEXT_GLYPH_POSITIONS_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/text/glyph_info.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_trans_affine.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <vector>
#include <list>

namespace mapnik {

struct glyph_position
{
    glyph_position(glyph_info const& _glyph, pixel_position const& _pos, rotation const& _rot)
        : glyph(_glyph)
        , pos(_pos)
        , rot(_rot)
    {}
    glyph_info const& glyph;
    pixel_position pos;
    rotation rot;
};

struct marker_info
{
    // marker_info() : marker(), transform() {}
    marker_info(std::shared_ptr<marker const> _marker, agg::trans_affine const& _transform)
        : marker_(_marker)
        , transform_(_transform)
    {}
    std::shared_ptr<marker const> marker_;
    agg::trans_affine transform_;
};

using marker_info_ptr = std::shared_ptr<marker_info>;

/** Stores positions of glphys.
 *
 * The actual glyphs and their format are stored in text_layout.
 */

class glyph_positions
{
  public:
    using const_iterator = std::vector<glyph_position>::const_iterator;
    glyph_positions();

    std::size_t size() const { return data_.size(); };
    const_iterator begin() const;
    const_iterator end() const;

    void emplace_back(glyph_info const& glyph, pixel_position offset, rotation const& rot);
    void reserve(unsigned count);

    pixel_position const& get_base_point() const;
    void set_base_point(pixel_position const& base_point);
    void set_marker(marker_info_ptr marker, pixel_position const& marker_pos);
    marker_info_ptr const& get_marker() const;
    pixel_position const& marker_pos() const;

  private:
    std::vector<glyph_position> data_;
    pixel_position base_point_;
    marker_info_ptr marker_info_;
    pixel_position marker_pos_;
    box2d<double> bbox_;
};

using glyph_positions_ptr = std::unique_ptr<glyph_positions>;

using placements_list = std::list<glyph_positions_ptr>;

struct scoped_glyph_positions_offset
{
    scoped_glyph_positions_offset(glyph_positions& glyphs, pixel_position const& offset)
        : glyphs_(glyphs)
        , base_point_(glyphs.get_base_point())
        , marker_pos_(glyphs.marker_pos())
    {
        // move the glyphs to the correct offset
        glyphs_.set_base_point(base_point_ + offset);

        // update the position of any marker
        if (auto const& marker_info = glyphs_.get_marker())
        {
            glyphs_.set_marker(marker_info, marker_pos_ + offset);
        }
    }

    ~scoped_glyph_positions_offset()
    {
        // set the base_point back how it was
        glyphs_.set_base_point(base_point_);

        // restore marker as well, if there is any
        if (auto const& marker_info = glyphs_.get_marker())
        {
            glyphs_.set_marker(marker_info, marker_pos_);
        }
    }

  private:
    glyph_positions& glyphs_;
    pixel_position base_point_;
    pixel_position marker_pos_;
};

} // namespace mapnik

#endif // MAPNIK_TEXT_GLYPH_POSITIONS_HPP
