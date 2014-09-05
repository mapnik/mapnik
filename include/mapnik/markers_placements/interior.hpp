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

#ifndef MAPNIK_MARKERS_PLACEMENTS_INTERIOR_HPP
#define MAPNIK_MARKERS_PLACEMENTS_INTERIOR_HPP

#include <mapnik/markers_placements/point.hpp>
#include <mapnik/geom_util.hpp>

namespace mapnik {

template <typename Locator, typename Detector>
class markers_interior_placement : public markers_point_placement<Locator, Detector>
{
public:
    markers_interior_placement(Locator &locator, Detector &detector, markers_placement_params const& params)
        : markers_point_placement<Locator, Detector>(locator, detector, params)
    {
    }

    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        if (this->done_)
        {
            return false;
        }

        if (this->locator_.type() == mapnik::geometry_type::types::Point)
        {
            return markers_point_placement<Locator, Detector>::get_point(x, y, angle, ignore_placement);
        }

        if (this->locator_.type() == mapnik::geometry_type::types::LineString)
        {
            if (!label::middle_point(this->locator_, x, y))
            {
                this->done_ = true;
                return false;
            }
        }
        else
        {
            if (!label::interior_position(this->locator_, x, y))
            {
                this->done_ = true;
                return false;
            }
        }

        angle = 0;

        box2d<double> box = this->perform_transform(angle, x, y);
        if (this->params_.avoid_edges && !this->detector_.extent().contains(box))
        {
            return false;
        }
        if (!this->params_.allow_overlap && !this->detector_.has_placement(box))
        {
            return false;
        }

        if (!ignore_placement)
        {
            this->detector_.insert(box);
        }

        this->done_ = true;
        return true;
    }
};

}

#endif // MAPNIK_MARKERS_PLACEMENTS_INTERIOR_HPP
