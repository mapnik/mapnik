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

// boost
#include <boost/foreach.hpp>
#include <boost/variant.hpp>

//std
#include <cmath>
#include <limits>

// stl
#include <vector>

using std::vector;

namespace mapnik
{

typedef box2d<double> bound_box;
typedef coord<double,2> layout_offset;
//typedef vector<bound_box> bound_box_vector;
//typedef vector<layout_offset> layout_vector<layout_offset>;

// This visitor will process offsets for the given layout
struct process_layout : public boost::static_visitor<>
{
    // The vector containing the existing, centered item bounding boxes
    const vector<bound_box> &member_boxes_;
    
    // The vector to populate with item offsets
    vector<layout_offset> &member_offsets_;
    
    process_layout(const vector<bound_box> &member_bboxes, 
                 vector<layout_offset> &member_offsets)
       : member_boxes_(member_bboxes), 
         member_offsets_(member_offsets)
    {
    }

    // Arrange group memebers in centered, horizontal row
    void operator()(simple_row_layout const& layout) const
    {
        double total_width = (member_boxes_.size() - 1) * layout.get_item_margin();
        BOOST_FOREACH(const bound_box &box, member_boxes_)
        {
            total_width += layout.get_item_margin();
            total_width += box.width();
        }
        
        double x_offset = -(total_width / 2.0);
        BOOST_FOREACH(const bound_box &box, member_boxes_)
        {
            member_offsets_.push_back(layout_offset(x_offset - box.minx(), 0.0));
            x_offset += box.width() + layout.get_item_margin();
        }
    }

    void operator()(pair_layout const& layout) const
    {
        // Implemention in progress
        member_offsets_.resize(member_boxes_.size());
    }
};

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
    
    void set_layout(const group_layout &layout)
    {
        layout_ = layout;
        update_layout_ = true;
    }
    
    void add_member_bound_box(const bound_box &member_box)
    {
        member_boxes_.push_back(member_box);
        update_layout_ = true;
    }
    
    const layout_offset &offset_at(size_t i)
    {
        handle_update();
        return member_offsets_.at(i);
    }
    
    const bound_box &offset_box_at(size_t i)
    {
        handle_update();
        const layout_offset &offset = member_offsets_.at(i);
        const bound_box &box = member_boxes_.at(i);
        return box2d<double>(box.minx() + offset.x, box.miny() + offset.y,
                             box.maxx() + offset.x, box.maxy() + offset.y);
    }
    
private:

    void handle_update()
    {
        if (update_layout_)
        {
            member_offsets_.clear();
            boost::apply_visitor(process_layout(member_boxes_, member_offsets_), layout_);
            update_layout_ = false;
        }     
    }

    group_layout layout_;
    vector<bound_box> member_boxes_;
    vector<layout_offset> member_offsets_;
    bool update_layout_;
};

}   // namespace mapnik

#endif // MAPNIK_GROUP_LAYOUT_MANAGER_HPP
