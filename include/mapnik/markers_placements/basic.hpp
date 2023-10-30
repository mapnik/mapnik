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

#ifndef MAPNIK_MARKERS_PLACEMENTS_BASIC_HPP
#define MAPNIK_MARKERS_PLACEMENTS_BASIC_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/util/math.hpp>
#include <mapnik/util/noncopyable.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_trans_affine.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

struct markers_placement_params
{
    box2d<double> size;
    agg::trans_affine tr;
    double spacing;
    double spacing_offset;
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
    {}

  protected:
    markers_placement_params const& params_;

    // Rotates the size_ box and translates the position.
    box2d<double> perform_transform(double angle, double dx, double dy) const
    {
        auto tr = params_.tr * agg::trans_affine_rotation(angle).translate(dx, dy);
        return box2d<double>(params_.size, tr);
    }

    bool set_direction(double& angle) const
    {
        switch (params_.direction)
        {
            case direction_enum::DIRECTION_UP:
                angle = 0;
                return true;
            case direction_enum::DIRECTION_DOWN:
                angle = util::pi;
                return true;
            case direction_enum::DIRECTION_AUTO:
                angle = util::normalize_angle(angle);
                if (std::abs(angle) > util::pi / 2)
                    angle += util::pi;
                return true;
            case direction_enum::DIRECTION_AUTO_DOWN:
                angle = util::normalize_angle(angle);
                if (std::abs(angle) < util::pi / 2)
                    angle += util::pi;
                return true;
            case direction_enum::DIRECTION_LEFT:
                angle += util::pi;
                return true;
            case direction_enum::DIRECTION_LEFT_ONLY:
                angle = util::normalize_angle(angle + util::pi);
                return std::fabs(angle) < util::pi / 2;
            case direction_enum::DIRECTION_RIGHT_ONLY:
                angle = util::normalize_angle(angle);
                return std::fabs(angle) < util::pi / 2;
            case direction_enum::DIRECTION_RIGHT:
            default:
                return true;
        }
    }
};

} // namespace mapnik

#endif // MAPNIK_MARKERS_PLACEMENTS_BASIC_HPP
