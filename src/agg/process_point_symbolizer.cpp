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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/renderer_common/process_point_symbolizer.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_trans_affine.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <string>

// boost

namespace mapnik {

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(point_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)
{
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);

    render_point_symbolizer(
        sym, feature, prj_trans, common_,
        [&](pixel_position const& pos, marker const& marker,
            agg::trans_affine const& tr, double opacity) {
            render_marker(pos, marker, tr, opacity, comp_op);
        });
}

template void agg_renderer<image_rgba8>::process(point_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
