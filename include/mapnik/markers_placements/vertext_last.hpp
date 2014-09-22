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

#ifndef MAPNIK_MARKERS_PLACEMENTS_VERTEXT_LAST_HPP
#define MAPNIK_MARKERS_PLACEMENTS_VERTEXT_LAST_HPP

#include <mapnik/markers_placements/point.hpp>

namespace mapnik {

template <typename Locator, typename Detector>
class markers_vertex_last_placement : public markers_point_placement<Locator, Detector>
{
public:
    markers_vertex_last_placement(Locator &locator, Detector &detector, markers_placement_params const& params)
        : markers_point_placement<Locator, Detector>(locator, detector, params) {}

    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        if (this->done_)
        {
            return false;
        }

        double x0, y0;
        unsigned command0 = this->locator_.vertex(&x0, &y0);

        if (agg::is_stop(command0))
        {
            this->done_ = true;
            return false;
        }

        double next_x, next_y;
        double x1 = x0, y1 = y0;
        unsigned command1 = command0;

        while (!agg::is_stop(command0 = this->locator_.vertex(&next_x, &next_y)))
        {
            command1 = command0;
            x1 = x0;
            y1 = y0;
            x0 = next_x;
            y0 = next_y;
        }

        x = x0;
        y = y0;

        if (agg::is_line_to(command1))
        {
            angle = std::atan2(y0 - y1, x0 - x1);
        }

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

#endif // MAPNIK_MARKERS_PLACEMENTS_VERTEXT_LAST_HPP
