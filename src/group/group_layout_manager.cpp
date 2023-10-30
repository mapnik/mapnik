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
#include <mapnik/group/group_layout_manager.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/safe_cast.hpp>

// std
#include <cmath>

namespace mapnik {

// This visitor will process offsets for the given layout
struct process_layout
{
    using bound_box = box2d<double>;

    // The vector containing the existing, centered item bounding boxes
    std::vector<bound_box> const& member_boxes_;

    // The vector to populate with item offsets
    std::vector<pixel_position>& member_offsets_;

    // The origin point of the member boxes
    // i.e. The member boxes are positioned around input_origin,
    //      and the offset values should position them around (0,0)
    pixel_position const& input_origin_;

    process_layout(std::vector<bound_box> const& member_bboxes,
                   std::vector<pixel_position>& member_offsets,
                   pixel_position const& input_origin)
        : member_boxes_(member_bboxes)
        , member_offsets_(member_offsets)
        , input_origin_(input_origin)
    {}

    // arrange group members in centered, horizontal row
    void operator()(simple_row_layout const& layout) const
    {
        member_offsets_.clear();
        member_offsets_.reserve(member_boxes_.size());

        double total_width = (member_boxes_.size() - 1) * layout.get_item_margin();
        for (auto const& box : member_boxes_)
        {
            total_width += box.width();
        }

        double x_offset = -(total_width / 2.0);
        for (auto const& box : member_boxes_)
        {
            member_offsets_.emplace_back(x_offset - box.minx(), -input_origin_.y);
            x_offset += box.width() + layout.get_item_margin();
        }
    }

    // arrange group members in x horizontal pairs of 2,
    // one to the left and one to the right of center in each pair
    void operator()(pair_layout const& layout) const
    {
        member_offsets_.resize(member_boxes_.size());
        double y_margin = layout.get_item_margin();
        double x_margin = y_margin / 2.0;

        if (member_boxes_.size() == 1)
        {
            member_offsets_[0] = pixel_position(0, 0) - input_origin_;
            return;
        }

        auto max_diff = layout.get_max_difference();
        auto layout_box = make_horiz_pair(0, 0.0, 0, x_margin, max_diff);
        auto y_shift = 0.5 * layout_box.height();

        for (size_t i = 2; i < member_boxes_.size(); i += 2)
        {
            auto y = layout_box.maxy() + y_margin;
            auto pair_box = make_horiz_pair(i, y, 1, x_margin, max_diff);
            layout_box.expand_to_include(pair_box);
        }

        // layout_box.center corresponds to the center of the first row;
        // shift offsets so that the whole group is centered vertically

        y_shift -= 0.5 * layout_box.height();

        for (auto& offset : member_offsets_)
        {
            offset.y += y_shift;
        }
    }

  private:

    // Place member bound boxes at [ifirst] and [ifirst + 1] in a horizontal pairi, vertically
    //   align with pair_y, store corresponding offsets, and return bound box of combined pair
    // Note: x_margin is the distance between box edge and x center
    bound_box make_horiz_pair(size_t ifirst, double pair_y, int y_dir, double x_margin, double max_diff) const
    {
        // two boxes available for pair
        if (ifirst + 1 < member_boxes_.size())
        {
            double x_center = member_boxes_[ifirst].width() - member_boxes_[ifirst + 1].width();
            if (max_diff < 0.0 || std::abs(x_center) <= max_diff)
            {
                x_center = 0.0;
            }

            bound_box pair_box = box_offset_align(ifirst, x_center - x_margin, pair_y, -1, y_dir);
            pair_box.expand_to_include(box_offset_align(ifirst + 1, x_center + x_margin, pair_y, 1, y_dir));
            return pair_box;
        }

        // only one box available for this "pair", so keep x-centered and handle y-offset
        return box_offset_align(ifirst, 0, pair_y, 0, y_dir);
    }

    // Offsets member bound box at [i] and align with (x, y), in direction <x_dir, y_dir>
    // stores corresponding offset, and returns modified bounding box
    bound_box box_offset_align(size_t i, double x, double y, int x_dir, int y_dir) const
    {
        auto box = member_boxes_[i];
        auto& offset = member_offsets_[i];
        offset.x = x - (x_dir == 0 ? input_origin_.x : (x_dir < 0 ? box.maxx() : box.minx()));
        offset.y = y - (y_dir == 0 ? input_origin_.y : (y_dir < 0 ? box.maxy() : box.miny()));
        box.move(offset.x, offset.y);
        return box;
    }
};

box2d<double> group_layout_manager::offset_box_at(size_t i)
{
    handle_update();
    pixel_position const& offset = member_offsets_.at(i);
    bound_box const& box = member_boxes_.at(i);
    return box2d<double>(box.minx() + offset.x, box.miny() + offset.y, box.maxx() + offset.x, box.maxy() + offset.y);
}

void group_layout_manager::handle_update()
{
    if (update_layout_)
    {
        member_offsets_.clear();
        util::apply_visitor(process_layout(member_boxes_, member_offsets_, input_origin_), layout_);
        update_layout_ = false;
    }
}

} // namespace mapnik
