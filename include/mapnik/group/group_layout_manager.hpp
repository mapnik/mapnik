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

#ifndef MAPNIK_GROUP_LAYOUT_MANAGER_HPP
#define MAPNIK_GROUP_LAYOUT_MANAGER_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/group/group_layout.hpp>

// stl
#include <vector>

namespace mapnik {

struct group_layout_manager
{
    using bound_box = box2d<double>;

    group_layout_manager()
        : update_layout_(false)
    {}

    explicit group_layout_manager(group_layout const& layout)
        : layout_(layout)
        , update_layout_(false)
    {}

    group_layout_manager(group_layout const& layout, pixel_position const& input_origin)
        : layout_(layout)
        , input_origin_(input_origin)
        , update_layout_(false)
    {}

    group_layout_manager(group_layout const& layout,
                         pixel_position const& input_origin,
                         std::vector<bound_box> const& item_boxes)
        : layout_(layout)
        , input_origin_(input_origin)
        , member_boxes_(item_boxes)
        , update_layout_(true)
    {}

    void set_input_origin(double x, double y)
    {
        input_origin_.set(x, y);
        update_layout_ = true;
    }

    void set_input_origin(pixel_position const& input_origin)
    {
        input_origin_ = input_origin;
        update_layout_ = true;
    }

    inline void set_layout(group_layout const& layout)
    {
        layout_ = layout;
        update_layout_ = true;
    }

    inline void add_member_bound_box(bound_box const& member_box)
    {
        member_boxes_.push_back(member_box);
        update_layout_ = true;
    }

    inline pixel_position const& offset_at(size_t i)
    {
        handle_update();
        return member_offsets_.at(i);
    }

    bound_box offset_box_at(size_t i);

  private:

    void handle_update();

    group_layout layout_;
    pixel_position input_origin_;
    std::vector<bound_box> member_boxes_;
    std::vector<pixel_position> member_offsets_;
    bool update_layout_;
};

} // namespace mapnik

#endif // MAPNIK_GROUP_LAYOUT_MANAGER_HPP
