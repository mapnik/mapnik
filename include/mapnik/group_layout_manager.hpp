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

#ifndef MAPNIK_GROUP_LAYOUT_MANAGER_HPP
#define MAPNIK_GROUP_LAYOUT_MANAGER_HPP

// mapnik
#include <mapnik/coord.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/group_layout.hpp>

// stl
#include <vector>

using std::vector;

namespace mapnik
{

typedef box2d<double> bound_box;
typedef coord<double,2> layout_offset;

struct MAPNIK_DECL group_layout_manager
{    
    group_layout_manager(const group_layout &layout)
        : layout_(layout),
          member_boxes_(vector<bound_box>()),
          member_offsets_(vector<layout_offset>()),
          update_layout_(true)
    {
    }
    
    group_layout_manager(const group_layout &layout, const vector<bound_box> &item_boxes)
        : layout_(layout),
          member_boxes_(item_boxes),
          member_offsets_(vector<layout_offset>()),
          update_layout_(true)
    {
    }
    
    inline void set_layout(const group_layout &layout)
    {
        layout_ = layout;
        update_layout_ = true;
    }
    
    inline void add_member_bound_box(const bound_box &member_box)
    {
        member_boxes_.push_back(member_box);
        update_layout_ = true;
    }
    
    inline const layout_offset &offset_at(size_t i)
    {
        handle_update();
        return member_offsets_.at(i);
    }
    
    bound_box offset_box_at(size_t i);
    
private:

    void handle_update();

    group_layout layout_;
    vector<bound_box> member_boxes_;
    vector<layout_offset> member_offsets_;
    bool update_layout_;
};

}   // namespace mapnik

#endif // MAPNIK_GROUP_LAYOUT_MANAGER_HPP
