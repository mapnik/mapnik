/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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

#ifndef MAPNIK_MARKERS_PLACEMENTS_BASIC_HPP
#define MAPNIK_MARKERS_PLACEMENTS_BASIC_HPP

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/util/math.hpp>
#include <mapnik/util/noncopyable.hpp>

// agg
#include "agg_basics.h"
#include "agg_trans_affine.h"

namespace mapnik {

struct markers_placement_params
{
    box2d<double> size;
    agg::trans_affine tr;
    double spacing;
    double max_error;
    bool allow_overlap;
    bool avoid_edges;
    direction_enum direction;
};

template <typename Locator, typename Detector>
class markers_basic_placement : util::noncopyable
{
public:
    markers_basic_placement(Locator & locator, Detector & detector,
                            markers_placement_params const& params)
        : locator_(locator),
          detector_(detector),
          params_(params),
          done_(false)
    {
        // no need to rewind locator here, markers_placement_finder
        // does that after construction
    }

    markers_basic_placement(markers_basic_placement && ) = default;

    virtual ~markers_basic_placement()
    {
        // empty but necessary
    }

    // Start again at first marker. Returns the same list of markers only works when they were NOT added to the detector.
    virtual void rewind()
    {
        locator_.rewind(0);
        done_ = false;
    }

    // Get next point where the marker should be placed. Returns true if a place is found, false if none is found.
    virtual bool get_point(double &x, double &y, double &angle, bool ignore_placement) = 0;

protected:
    Locator & locator_;
    Detector & detector_;
    markers_placement_params const& params_;
    bool done_;

    // Rotates the size_ box and translates the position.
    box2d<double> perform_transform(double angle, double dx, double dy) const
    {
        auto tr = params_.tr * agg::trans_affine_rotation(angle).translate(dx, dy);
        return box2d<double>(params_.size, tr);
    }

    // Checks transformed box placement with collision detector.
    // returns false if the box:
    //  - a) isn't wholly inside extent and avoid_edges == true
    //  - b) collides with something and allow_overlap == false
    // otherwise returns true, and if ignore_placement == true,
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

    bool set_direction(double & angle) const
    {
        switch (params_.direction)
        {
            case DIRECTION_UP:
                angle = 0;
                return true;
            case DIRECTION_DOWN:
                angle = M_PI;
                return true;
            case DIRECTION_AUTO:
                if (std::fabs(util::normalize_angle(angle)) > 0.5 * M_PI)
                    angle += M_PI;
                return true;
            case DIRECTION_AUTO_DOWN:
                if (std::fabs(util::normalize_angle(angle)) < 0.5 * M_PI)
                    angle += M_PI;
                return true;
            case DIRECTION_LEFT:
                angle += M_PI;
                return true;
            case DIRECTION_LEFT_ONLY:
                angle += M_PI;
                return std::fabs(util::normalize_angle(angle)) < 0.5 * M_PI;
            case DIRECTION_RIGHT_ONLY:
                return std::fabs(util::normalize_angle(angle)) < 0.5 * M_PI;
            case DIRECTION_RIGHT:
            default:
                return true;
        }
    }
};

} // namespace mapnik

#endif // MAPNIK_MARKERS_PLACEMENTS_BASIC_HPP
