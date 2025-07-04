/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/image_scaling.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/config.hpp>
#include <mapnik/renderer_common/process_raster_symbolizer.hpp>

// stl
#include <cmath>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

template<typename T0, typename T1>
void agg_renderer<T0, T1>::process(raster_symbolizer const& sym,
                                   mapnik::feature_impl& feature,
                                   proj_transform const& prj_trans)
{
    render_raster_symbolizer(
      sym,
      feature,
      prj_trans,
      common_,
      [&](image_rgba8 const& target, composite_mode_e comp_op, double opacity, int start_x, int start_y) {
          composite(buffers_.top().get(), target, comp_op, opacity, start_x, start_y);
      });
}

template void
  agg_renderer<image_rgba8>::process(raster_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik
