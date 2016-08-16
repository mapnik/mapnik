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

#ifndef MAPNIK_GRID_MARKER_HELPERS_HPP
#define MAPNIK_GRID_MARKER_HELPERS_HPP

// mapnik
#include <mapnik/feature.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_gray.h"
#pragma GCC diagnostic pop

namespace mapnik {

template <typename RendererType, typename RasterizerType>
void render_raster_marker(RendererType ren,
                          RasterizerType & ras,
                          image_rgba8 const& src,
                          mapnik::feature_impl const& feature,
                          agg::trans_affine const& marker_tr,
                          double opacity)
{
    using color_type = typename RendererType::color_type;
    agg::scanline_bin sl;
    double width  = src.width();
    double height = src.height();
    double p[8];
    p[0] = 0;     p[1] = 0;
    p[2] = width; p[3] = 0;
    p[4] = width; p[5] = height;
    p[6] = 0;     p[7] = height;
    marker_tr.transform(&p[0], &p[1]);
    marker_tr.transform(&p[2], &p[3]);
    marker_tr.transform(&p[4], &p[5]);
    marker_tr.transform(&p[6], &p[7]);
    ras.move_to_d(p[0],p[1]);
    ras.line_to_d(p[2],p[3]);
    ras.line_to_d(p[4],p[5]);
    ras.line_to_d(p[6],p[7]);
    ren.color(color_type(feature.id()));
    agg::render_scanlines(ras, sl, ren);
}

}

#endif
