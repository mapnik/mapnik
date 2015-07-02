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


#ifndef MAPNIK_PATTERN_ALIGNMENT_HPP
#define MAPNIK_PATTERN_ALIGNMENT_HPP

#include <mapnik/geometry.hpp>

namespace mapnik { namespace detail {

struct apply_local_alignment
{
    apply_local_alignment(view_transform const& t,
                          proj_transform const& prj_trans,
                          box2d<double> const& clip_box,
                          double & x, double & y)
        : t_(t),
          prj_trans_(prj_trans),
          clip_box_(clip_box),
          x_(x),
          y_(y) {}
    
    void operator() (geometry::polygon_vertex_adapter<double> & va)
    {
        using clipped_geometry_type = agg::conv_clip_polygon<geometry::polygon_vertex_adapter<double> >;
        using path_type = transform_path_adapter<view_transform,clipped_geometry_type>;
        clipped_geometry_type clipped(va);
        clipped.clip_box(clip_box_.minx(),clip_box_.miny(),clip_box_.maxx(),clip_box_.maxy());
        path_type path(t_, clipped, prj_trans_);
        path.vertex(&x_,&y_);
    }

    template <typename Adapter>
    void operator() (Adapter &)
    {
        // no-op
    }

    view_transform const& t_;
    proj_transform const& prj_trans_;
    box2d<double> const& clip_box_;
    double & x_;
    double & y_;
};

}}

#endif // MAPNIK_PATTERN_ALIGNMENT_HPP
