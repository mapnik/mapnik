/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/util/math.hpp>
#include <mapnik/util/noncopyable.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_trans_affine.h"
#pragma GCC diagnostic pop

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
    double scale_factor;
};

class markers_basic_placement : util::noncopyable
{
public:
    markers_basic_placement(markers_placement_params const& params)
        : params_(params)
    {
    }

protected:
    markers_placement_params const& params_;

    // Rotates the size_ box and translates the position.
    box2d<double> perform_transform(double angle, double dx, double dy) const
    {
        auto tr = params_.tr * agg::trans_affine_rotation(angle).translate(dx, dy);
        return box2d<double>(params_.size, tr);
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
