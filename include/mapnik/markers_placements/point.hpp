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

#ifndef MAPNIK_MARKERS_PLACEMENTS_POINT_HPP
#define MAPNIK_MARKERS_PLACEMENTS_POINT_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/geometry/geometry_types.hpp>
#include <mapnik/markers_placements/basic.hpp>

namespace mapnik {

template <typename Locator, typename Detector>
class markers_point_placement : public markers_basic_placement
{
public:
    markers_point_placement(Locator & locator, Detector & detector,
                            markers_placement_params const& params)
        : markers_basic_placement(params),
          locator_(locator),
          detector_(detector),
          done_(false),
          use_angle_(false)
    {
        locator_.rewind(0);
    }

    // Use angle of line
    void use_angle(bool enable)
    {
        use_angle_ = enable;
    }

    // Start again at first marker. Returns the same list of markers only works when they were NOT added to the detector.
    void rewind()
    {
        locator_.rewind(0);
        done_ = false;
    }

    // Get next point where the marker should be placed. Returns true if a place is found, false if none is found.
    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        if (this->done_)
        {
            return false;
        }

        angle = 0;

        if (this->locator_.type() == geometry::geometry_types::LineString)
        {
            if (!label::middle_point(this->locator_, x, y, use_angle_ ? boost::optional<double&>(angle) : boost::none))
            {
                this->done_ = true;
                return false;
            }
        }
        else
        {
            if (!label::centroid(this->locator_, x, y))
            {
                this->done_ = true;
                return false;
            }
        }

        if (!this->push_to_detector(x, y, angle, ignore_placement))
        {
            return false;
        }

        this->done_ = true;
        return true;
    }

protected:
    Locator & locator_;
    Detector & detector_;
    bool done_;
    bool use_angle_;

    // Checks transformed box placement with collision detector.
    // returns false if the box:
    //  - a) isn't wholly inside extent and avoid_edges == true
    //  - b) collides with something and allow_overlap == false
    // otherwise returns true, and if ignore_placement == false,
    //  also adds the box to collision detector
    bool push_to_detector(double x, double y, double angle, bool ignore_placement)
    {
        auto box = perform_transform(angle, x, y);
        if (params_.avoid_edges && !detector_.extent().contains(box))
        {
            return false;
        }
        if (!params_.allow_overlap && !detector_.has_placement(box))
        {
            return false;
        }
        if (!ignore_placement)
        {
            detector_.insert(box);
        }
        return true;
    }
};

}

#endif // MAPNIK_MARKERS_PLACEMENTS_POINT_HPP
