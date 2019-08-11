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

#ifndef MAPNIK_MARKERS_PLACEMENT_HPP
#define MAPNIK_MARKERS_PLACEMENT_HPP

#include <mapnik/markers_placements/line.hpp>
#include <mapnik/markers_placements/point.hpp>
#include <mapnik/markers_placements/interior.hpp>
#include <mapnik/markers_placements/vertex_first.hpp>
#include <mapnik/markers_placements/vertex_last.hpp>
#include <mapnik/markers_placements/polylabel.hpp>
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
        : placement_type_(placement_type)
    {
        switch (marker_placement_enum(placement_type))
        {
        default:
        case MARKER_POINT_PLACEMENT:
            construct(&point_, locator, detector, params);
            break;
        case MARKER_ANGLED_POINT_PLACEMENT:
            construct(&point_, locator, detector, params);
            point_.use_angle(true);
            break;
        case MARKER_INTERIOR_PLACEMENT:
            construct(&interior_, locator, detector, params);
            break;
        case MARKER_LINE_PLACEMENT:
            construct(&line_, locator, detector, params);
            break;
        case MARKER_VERTEX_FIRST_PLACEMENT:
            construct(&vertex_first_, locator, detector, params);
            break;
        case MARKER_VERTEX_LAST_PLACEMENT:
            construct(&vertex_last_, locator, detector, params);
            break;
        case MARKER_POLYLABEL_PLACEMENT:
            construct(&polylabel_, locator, detector, params);
            break;
        }
    }

    ~markers_placement_finder()
    {
        switch (marker_placement_enum(placement_type_))
        {
        default:
        case MARKER_POINT_PLACEMENT:
        case MARKER_ANGLED_POINT_PLACEMENT:
            destroy(&point_);
            break;
        case MARKER_INTERIOR_PLACEMENT:
            destroy(&interior_);
            break;
        case MARKER_LINE_PLACEMENT:
            destroy(&line_);
            break;
        case MARKER_VERTEX_FIRST_PLACEMENT:
            destroy(&vertex_first_);
            break;
        case MARKER_VERTEX_LAST_PLACEMENT:
            destroy(&vertex_last_);
            break;
        case MARKER_POLYLABEL_PLACEMENT:
            destroy(&polylabel_);
            break;
        }
    }

    // Get next point where the marker should be placed. Returns true if a place is found, false if none is found.
    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        switch (marker_placement_enum(placement_type_))
        {
        default:
        case MARKER_POINT_PLACEMENT:
        case MARKER_ANGLED_POINT_PLACEMENT:
            return point_.get_point(x, y, angle, ignore_placement);
        case MARKER_INTERIOR_PLACEMENT:
            return interior_.get_point(x, y, angle, ignore_placement);
        case MARKER_LINE_PLACEMENT:
            return line_.get_point(x, y, angle, ignore_placement);
        case MARKER_VERTEX_FIRST_PLACEMENT:
            return vertex_first_.get_point(x, y, angle, ignore_placement);
        case MARKER_VERTEX_LAST_PLACEMENT:
            return vertex_last_.get_point(x, y, angle, ignore_placement);
        case MARKER_POLYLABEL_PLACEMENT:
            return polylabel_.get_point(x, y, angle, ignore_placement);
        }
    }

private:
    marker_placement_e const placement_type_;

    union
    {
        markers_point_placement<Locator, Detector> point_;
        markers_line_placement<Locator, Detector> line_;
        markers_interior_placement<Locator, Detector> interior_;
        markers_vertex_first_placement<Locator, Detector> vertex_first_;
        markers_vertex_last_placement<Locator, Detector> vertex_last_;
        markers_polylabel_placement<Locator, Detector> polylabel_;
    };

    template <typename T>
    static T* construct(T* what, Locator & locator, Detector & detector,
                        markers_placement_params const& params)
    {
        return new(what) T(locator, detector, params);
    }

    template <typename T>
    static void destroy(T* what)
    {
        what->~T();
    }
};

}

#endif // MAPNIK_MARKERS_PLACEMENT_HPP
