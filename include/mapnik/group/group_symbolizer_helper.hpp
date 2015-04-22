/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#ifndef GROUP_SYMBOLIZER_HELPER_HPP
#define GROUP_SYMBOLIZER_HELPER_HPP

//mapnik
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/pixel_position.hpp>

namespace mapnik {

class label_collision_detector4;
class feature_impl;
class proj_transform;
class view_transform;
using DetectorType = label_collision_detector4;

using pixel_position_list = std::list<pixel_position>;

// Helper object that does some of the GroupSymbolizer placement finding work.
class group_symbolizer_helper : public base_symbolizer_helper
{
public:
    struct box_element
    {
        box_element(box2d<double> const& box, value_unicode_string const& repeat_key = "")
           : box_(box),
             repeat_key_(repeat_key)
        {}
        box2d<double> box_;
        value_unicode_string repeat_key_;
    };

    group_symbolizer_helper(group_symbolizer const& sym,
                            feature_impl const& feature,
                            attributes const& vars,
                            proj_transform const& prj_trans,
                            unsigned width,
                            unsigned height,
                            double scale_factor,
                            view_transform const& t,
                            DetectorType &detector,
                            box2d<double> const& query_extent);

    inline void add_box_element(box2d<double> const& box, value_unicode_string const& repeat_key = "")
    {
        box_elements_.push_back(box_element(box, repeat_key));
    }

    inline void clear_box_elements()
    {
        box_elements_.clear();
    }

    inline text_symbolizer_properties const& get_properties() const
    {
        return info_ptr_->properties;
    }

    pixel_position_list const& get();

    // Iterate over the given path, placing line-following labels or point labels with respect to label_spacing.
    template <typename T>
    bool find_line_placements(T & path);
private:


    // Check if a point placement fits at given position
    bool check_point_placement(pixel_position const& pos);
    // Checks for collision.
    bool collision(box2d<double> const& box, value_unicode_string const& repeat_key = "") const;
    double get_spacing(double path_length) const;

    DetectorType & detector_;

    // Boxes and repeat keys to take into account when finding placement.
    //  Boxes are relative to starting point of current placement.
    //
    std::list<box_element> box_elements_;

    pixel_position_list results_;
};

} //namespace
#endif // GROUP_SYMBOLIZER_HELPER_HPP
