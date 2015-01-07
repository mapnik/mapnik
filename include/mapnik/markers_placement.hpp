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

#ifndef MAPNIK_MARKERS_PLACEMENT_HPP
#define MAPNIK_MARKERS_PLACEMENT_HPP

#include <mapnik/markers_placements/line.hpp>
#include <mapnik/markers_placements/point.hpp>
#include <mapnik/markers_placements/interior.hpp>
#include <mapnik/markers_placements/vertext_first.hpp>
#include <mapnik/markers_placements/vertext_last.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik
{

template <typename Locator, typename Detector>
class markers_placement_finder : util::noncopyable
{
public:
    using markers_placement = util::variant<markers_point_placement<Locator, Detector>,
                                            markers_line_placement<Locator, Detector>,
                                            markers_interior_placement<Locator, Detector>,
                                            markers_vertex_first_placement<Locator, Detector>,
                                            markers_vertex_last_placement<Locator, Detector>>;

    class get_point_visitor
    {
    public:
        get_point_visitor(double &x, double &y, double &angle, bool ignore_placement)
            : x_(x), y_(y), angle_(angle), ignore_placement_(ignore_placement)
        {
        }

        template <typename T>
        bool operator()(T &placement) const
        {
            return placement.get_point(x_, y_, angle_, ignore_placement_);
        }

    private:
        double &x_, &y_, &angle_;
        bool ignore_placement_;
    };

    markers_placement_finder(marker_placement_e placement_type,
                             Locator &locator,
                             Detector &detector,
                             markers_placement_params const& params)
        : placement_(create(placement_type, locator, detector, params))
    {
    }

    // Get next point where the marker should be placed. Returns true if a place is found, false if none is found.
    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        return util::apply_visitor(get_point_visitor(x, y, angle, ignore_placement), placement_);
    }

private:
    // Factory function for particular placement implementations.
    static markers_placement create(marker_placement_e placement_type,
                                    Locator &locator,
                                    Detector &detector,
                                    markers_placement_params const& params)
    {
        switch (placement_type)
        {
        case MARKER_POINT_PLACEMENT:
            return markers_point_placement<Locator, Detector>(locator,detector,params);
        case MARKER_INTERIOR_PLACEMENT:
            return markers_interior_placement<Locator, Detector>(locator,detector,params);
        case MARKER_LINE_PLACEMENT:
            return markers_line_placement<Locator, Detector>(locator,detector,params);
        case MARKER_VERTEX_FIRST_PLACEMENT:
            return markers_vertex_first_placement<Locator, Detector>(locator,detector,params);
        case MARKER_VERTEX_LAST_PLACEMENT:
            return markers_vertex_last_placement<Locator, Detector>(locator,detector,params);
        default: // point
            return markers_point_placement<Locator, Detector>(locator,detector,params);
        }
    }

    markers_placement placement_;
};

}

#endif // MAPNIK_MARKERS_PLACEMENT_HPP
