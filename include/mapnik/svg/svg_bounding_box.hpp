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

#ifndef MAPNIK_SVG_BOUNDING_BOX_HPP
#define MAPNIK_SVG_BOUNDING_BOX_HPP

#include <mapnik/svg/svg_group.hpp>
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace svg {
namespace detail {
template <typename VertexSource>
struct bounding_box
{
    bounding_box(VertexSource& vs, double& x1, double& y1, double& x2, double& y2, bool& first)
        : vs_(vs), x1_(x1), y1_(y1), x2_(x2), y2_(y2), first_(first) {}

    void operator() (group const& g) const
    {
        for (auto const& elem : g.elements)
        {
            mapbox::util::apply_visitor(bounding_box(vs_, x1_, y1_, x2_, y2_, first_), elem);
        }
    }
    void operator() (path_attributes const& attr) const
    {
        vs_.rewind(attr.index);
        vs_.transformer(const_cast<agg::trans_affine&>(attr.transform));
        unsigned cmd;
        double x;
        double y;
        while(!is_stop(cmd = vs_.vertex(&x, &y)))
        {
            if(is_vertex(cmd))
            {
                if (first_)
                {
                    x1_ = x;
                    y1_ = y;
                    x2_ = x;
                    y2_ = y;
                    first_ = false;
                }
                else
                {
                    if (x < x1_) x1_ = x;
                    if (y < y1_) y1_ = y;
                    if (x > x2_) x2_ = x;
                    if (y > y2_) y2_ = y;
                }
            }
        }
    }
    VertexSource& vs_;
    double& x1_;
    double& y1_;
    double& x2_;
    double& y2_;
    bool& first_;
};
} // detail

template <typename VertexSource>
void bounding_box(VertexSource& vs, group const& g, double& x1, double& y1, double& x2, double& y2)
{
    bool first = true;
    x1 = 1.0;
    y1 = 1.0;
    x2 = 0.0;
    y2 = 0.0; // ^^ FIXME: AGG specific "invalid bbox" logic
    for (auto const& elem : g.elements)
    {
        mapbox::util::apply_visitor(detail::bounding_box<VertexSource>(vs, x1, y1, x2, y2, first), elem);
    }
}

} // svg
} // mapnik

#endif //MAPNIK_SVG_BOUNDING_BOX_HPP
