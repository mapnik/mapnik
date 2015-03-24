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

#ifndef MAPNIK_MARKERS_PLACEMENTS_LINE_HPP
#define MAPNIK_MARKERS_PLACEMENTS_LINE_HPP

#include <mapnik/markers_placements/point.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/tolerance_iterator.hpp>
#include <mapnik/geometry_types.hpp>

namespace mapnik {

template <typename Locator, typename Detector>
class markers_line_placement : public markers_point_placement<Locator, Detector>
{
public:
    markers_line_placement(Locator &locator, Detector &detector, markers_placement_params const& params)
        : markers_point_placement<Locator, Detector>(locator, detector, params),
            first_point_(true),
            spacing_(0.0),
            marker_width_((params.size * params.tr).width()),
            path_(locator)
    {
        spacing_ = params.spacing < 1 ? 100 : params.spacing;
        rewind();
    }

    markers_line_placement(markers_line_placement && rhs)
        : markers_point_placement<Locator, Detector>(std::move(rhs)),
          first_point_(std::move(rhs.first_point_)),
          spacing_(std::move(rhs.spacing_)),
          marker_width_(std::move(rhs.marker_width_)),
          path_(std::move(rhs.path_))
    {}

    void rewind()
    {
        this->locator_.rewind(0);
        this->done_ = false;
        first_point_ = true;
    }

    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        if (this->done_)
        {
            return false;
        }

        if (this->locator_.type() == geometry::geometry_types::Point)
        {
            return markers_point_placement<Locator, Detector>::get_point(x, y, angle, ignore_placement);
        }

        double move = spacing_;

        if (first_point_)
        {
            if (!path_.next_subpath())
            {
                this->done_ = true;
                return false;
            }
            first_point_ = false;
            move = spacing_ / 2.0;
        }

        while (path_.forward(move))
        {
            tolerance_iterator tolerance_offset(spacing_ * this->params_.max_error, 0.0);
            while (tolerance_offset.next())
            {
                vertex_cache::scoped_state state(path_);
                if (path_.move(tolerance_offset.get()) && (path_.linear_position() + marker_width_ / 2.0) < path_.length())
                {
                    pixel_position pos = path_.current_position();
                    x = pos.x;
                    y = pos.y;
                    angle = path_.current_segment_angle();
                    if (!this->set_direction(angle))
                    {
                        continue;
                    }
                    box2d<double> box = this->perform_transform(angle, x, y);
                    if ((this->params_.avoid_edges && !this->detector_.extent().contains(box))
                        || (!this->params_.allow_overlap && !this->detector_.has_placement(box)))
                    {
                        continue;
                    }
                    if (!ignore_placement)
                    {
                        this->detector_.insert(box);
                    }
                    return true;
                }
            }
        }

        this->done_ = true;

        return false;
    }

private:
    bool first_point_;
    double spacing_;
    double marker_width_;
    vertex_cache path_;
};

}

#endif // MAPNIK_MARKERS_PLACEMENTS_LINE_HPP
