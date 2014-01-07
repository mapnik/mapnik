/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>

#include <mapnik/geom_util.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/renderer_common/process_point_symbolizer.hpp>

// agg
#include "agg_trans_affine.h"

// stl
#include <string>

// boost

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(point_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, src_over);

    render_point_symbolizer(
        sym, feature, prj_trans, common_,
        [&](pixel_position const &pos, marker const &marker,
            agg::trans_affine const &tr, double opacity) {
                render_marker(feature,
                              pixmap_.get_resolution(),
                              pos,
                              marker,
                              tr,
                              opacity,
                              comp_op);
        });
}

template void grid_renderer<grid>::process(point_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}
