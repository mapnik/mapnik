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

#ifndef MAPNIK_MARKERS_PLACEMENTS_VERTEXT_LAST_HPP
#define MAPNIK_MARKERS_PLACEMENTS_VERTEXT_LAST_HPP

#include <mapnik/markers_placements/point.hpp>

namespace mapnik {

template <typename Locator, typename Detector>
class markers_vertex_last_placement : public markers_point_placement<Locator, Detector>
{
public:
    using point_placement = markers_point_placement<Locator, Detector>;
    using point_placement::point_placement;

    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        if (this->done_)
        {
            return false;
        }

        geometry::point<double> p0;
        geometry::point<double> p1;
        geometry::point<double> p_next;
        geometry::point<double> move_to;
        unsigned cmd0 = SEG_END;
        unsigned cmd1 = SEG_END;
        unsigned cmd_next = SEG_END;

        while ((cmd_next = this->locator_.vertex(&p_next.x, &p_next.y)) != SEG_END)
        {
            switch (cmd_next)
            {
                case SEG_MOVETO:
                    move_to = p_next;
                    p0 = p_next;
                    cmd0 = cmd_next;
                    break;
                case SEG_LINETO:
                    p1 = p0;
                    cmd1 = cmd0;
                    p0 = p_next;
                    cmd0 = cmd_next;
                    break;
                case SEG_CLOSE:
                    p1 = p0;
                    cmd1 = cmd0;
                    p0 = move_to;
                    cmd0 = cmd_next;
                    break;
            }
        }

        // Empty geometry
        if (cmd0 == SEG_END)
        {
            this->done_ = true;
            return false;
        }

        // Last point
        x = p0.x;
        y = p0.y;

        // Line or polygon
        if (cmd0 == SEG_LINETO || cmd0 == SEG_CLOSE)
        {
            angle = std::atan2(p0.y - p1.y, p0.x - p1.x);
            if (!this->set_direction(angle))
            {
                this->done_ = true;
                return false;
            }
        }
        else
        {
            angle = 0.0;
        }

        if (!this->push_to_detector(x, y, angle, ignore_placement))
        {
            this->done_ = true;
            return false;
        }

        this->done_ = true;
        return true;
    }
};

}

#endif // MAPNIK_MARKERS_PLACEMENTS_VERTEXT_LAST_HPP
