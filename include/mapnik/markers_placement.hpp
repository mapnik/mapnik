/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_MARKERS_PLACEMENT_HPP
#define MAPNIK_MARKERS_PLACEMENT_HPP

#include <mapnik/markers_placements/line.hpp>
#include <mapnik/markers_placements/point.hpp>
#include <mapnik/markers_placements/interior.hpp>
#include <mapnik/markers_placements/vertext_first.hpp>
#include <mapnik/markers_placements/vertext_last.hpp>
#include <mapnik/symbolizer_enumerations.hpp>

namespace mapnik
{

template <typename Locator, typename Detector>
class markers_placement_finder : util::noncopyable
{
public:
    markers_placement_finder(marker_placement_e placement_type,
                             Locator &locator,
                             Detector &detector,
                             markers_placement_params const& params)
        : active_placement_(nullptr)
    {
        switch (placement_type)
        {
        default:
        case MARKER_POINT_PLACEMENT:
            active_placement_ = construct(&point_, locator, detector, params);
            break;
        case MARKER_INTERIOR_PLACEMENT:
            active_placement_ = construct(&interior_, locator, detector, params);
            break;
        case MARKER_LINE_PLACEMENT:
            active_placement_ = construct(&line_, locator, detector, params);
            break;
        case MARKER_VERTEX_FIRST_PLACEMENT:
            active_placement_ = construct(&vertex_first_, locator, detector, params);
            break;
        case MARKER_VERTEX_LAST_PLACEMENT:
            active_placement_ = construct(&vertex_last_, locator, detector, params);
            break;
        }
        // previously placement-type constructors (markers_*_placement)
        // rewound the locator; reasons for rewinding here instead:
        //  1) so that nobody is tempted to call now-virtual rewind()
        //      in placement-type class constructors
        //  2) it servers as a runtime check that the above switch isn't
        //      missing cases and active_placement_ points to an object
        active_placement_->rewind();
    }

    ~markers_placement_finder()
    {
        active_placement_->~markers_basic_placement();
    }

    // Get next point where the marker should be placed. Returns true if a place is found, false if none is found.
    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        return active_placement_->get_point(x, y, angle, ignore_placement);
    }

private:
    markers_basic_placement* active_placement_;

    union
    {
        markers_point_placement<Locator, Detector> point_;
        markers_line_placement<Locator, Detector> line_;
        markers_interior_placement<Locator, Detector> interior_;
        markers_vertex_first_placement<Locator, Detector> vertex_first_;
        markers_vertex_last_placement<Locator, Detector> vertex_last_;
    };

    template <typename T>
    static T* construct(T* what, Locator & locator, Detector & detector,
                        markers_placement_params const& params)
    {
        return new(what) T(locator, detector, params);
    }
};

}

#endif // MAPNIK_MARKERS_PLACEMENT_HPP
