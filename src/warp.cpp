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
#include <mapnik/warp.hpp>
#include <mapnik/config.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/span_image_filter.hpp>

// agg
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_basics.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_renderer_scanline.h"

namespace mapnik {

void reproject_and_scale_raster(raster & target, raster const& source,
                                proj_transform const& prj_trans,
                                double offset_x, double offset_y,
                                unsigned mesh_size,
                                double filter_radius,
                                scaling_method_e scaling_method)
{
    CoordTransform ts(source.data_.width(), source.data_.height(),
                      source.ext_);
    CoordTransform tt(target.data_.width(), target.data_.height(),
                      target.ext_, offset_x, offset_y);
    unsigned i, j;
    unsigned mesh_nx = ceil(source.data_.width()/double(mesh_size) + 1);
    unsigned mesh_ny = ceil(source.data_.height()/double(mesh_size) + 1);

    ImageData<double> xs(mesh_nx, mesh_ny);
    ImageData<double> ys(mesh_nx, mesh_ny);

    // Precalculate reprojected mesh
    for(j=0; j<mesh_ny; ++j)
    {
        for (i=0; i<mesh_nx; ++i)
        {
            xs(i,j) = std::min(i*mesh_size,source.data_.width());
            ys(i,j) = std::min(j*mesh_size,source.data_.height());
            ts.backward(&xs(i,j), &ys(i,j));
        }
    }
    prj_trans.backward(xs.getData(), ys.getData(), NULL, mesh_nx*mesh_ny);

    // Initialize AGG objects
    typedef agg::pixfmt_rgba32 pixfmt;
    typedef pixfmt::color_type color_type;
    typedef agg::renderer_base<pixfmt> renderer_base;
    typedef agg::pixfmt_rgba32_pre pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

    agg::rasterizer_scanline_aa<> rasterizer;
    agg::scanline_u8  scanline;
    agg::rendering_buffer buf((unsigned char*)target.data_.getData(),
                              target.data_.width(),
                              target.data_.height(),
                              target.data_.width()*4);
    pixfmt_pre pixf_pre(buf);
    renderer_base_pre rb_pre(pixf_pre);
    rasterizer.clip_box(0, 0, target.data_.width(), target.data_.height());
    agg::rendering_buffer buf_tile(
        (unsigned char*)source.data_.getData(),
        source.data_.width(),
        source.data_.height(),
        source.data_.width() * 4);

    pixfmt pixf_tile(buf_tile);

    typedef agg::image_accessor_clone<pixfmt> img_accessor_type;
    img_accessor_type ia(pixf_tile);

    agg::span_allocator<color_type> sa;

    // Initialize filter
    agg::image_filter_lut filter;
    switch(scaling_method)
    {
    case SCALING_NEAR: break;
    case SCALING_BILINEAR8: // TODO - impl this or remove?
    case SCALING_BILINEAR:
        filter.calculate(agg::image_filter_bilinear(), true); break;
    case SCALING_BICUBIC:
        filter.calculate(agg::image_filter_bicubic(), true); break;
    case SCALING_SPLINE16:
        filter.calculate(agg::image_filter_spline16(), true); break;
    case SCALING_SPLINE36:
        filter.calculate(agg::image_filter_spline36(), true); break;
    case SCALING_HANNING:
        filter.calculate(agg::image_filter_hanning(), true); break;
    case SCALING_HAMMING:
        filter.calculate(agg::image_filter_hamming(), true); break;
    case SCALING_HERMITE:
        filter.calculate(agg::image_filter_hermite(), true); break;
    case SCALING_KAISER:
        filter.calculate(agg::image_filter_kaiser(), true); break;
    case SCALING_QUADRIC:
        filter.calculate(agg::image_filter_quadric(), true); break;
    case SCALING_CATROM:
        filter.calculate(agg::image_filter_catrom(), true); break;
    case SCALING_GAUSSIAN:
        filter.calculate(agg::image_filter_gaussian(), true); break;
    case SCALING_BESSEL:
        filter.calculate(agg::image_filter_bessel(), true); break;
    case SCALING_MITCHELL:
        filter.calculate(agg::image_filter_mitchell(), true); break;
    case SCALING_SINC:
        filter.calculate(agg::image_filter_sinc(filter_radius), true); break;
    case SCALING_LANCZOS:
        filter.calculate(agg::image_filter_lanczos(filter_radius), true); break;
    case SCALING_BLACKMAN:
        filter.calculate(agg::image_filter_blackman(filter_radius), true); break;
    }

    // Project mesh cells into target interpolating raster inside each one
    for(j=0; j<mesh_ny-1; j++)
    {
        for (i=0; i<mesh_nx-1; i++)
        {
            double polygon[8] = {xs(i,j), ys(i,j),
                                 xs(i+1,j), ys(i+1,j),
                                 xs(i+1,j+1), ys(i+1,j+1),
                                 xs(i,j+1), ys(i,j+1)};
            tt.forward(polygon+0, polygon+1);
            tt.forward(polygon+2, polygon+3);
            tt.forward(polygon+4, polygon+5);
            tt.forward(polygon+6, polygon+7);

            rasterizer.reset();
            rasterizer.move_to_d(std::floor(polygon[0]), std::floor(polygon[1]));
            rasterizer.line_to_d(std::floor(polygon[2]), std::floor(polygon[3]));
            rasterizer.line_to_d(std::floor(polygon[4]), std::floor(polygon[5]));
            rasterizer.line_to_d(std::floor(polygon[6]), std::floor(polygon[7]));

            unsigned x0 = i * mesh_size;
            unsigned y0 = j * mesh_size;
            unsigned x1 = (i+1) * mesh_size;
            unsigned y1 = (j+1) * mesh_size;
            x1 = std::min(x1, source.data_.width());
            y1 = std::min(y1, source.data_.height());
            agg::trans_affine tr(polygon, x0, y0, x1, y1);
            if (tr.is_valid())
            {
                typedef agg::span_interpolator_linear<agg::trans_affine>
                    interpolator_type;
                interpolator_type interpolator(tr);

                if (scaling_method == SCALING_NEAR) {
                    typedef agg::span_image_filter_rgba_nn
                        <img_accessor_type, interpolator_type>
                        span_gen_type;
                    span_gen_type sg(ia, interpolator);
                    agg::render_scanlines_aa(rasterizer, scanline, rb_pre,
                                             sa, sg);
                } else {
                    typedef mapnik::span_image_resample_rgba_affine
                        <img_accessor_type> span_gen_type;
                    span_gen_type sg(ia, interpolator, filter);
                    agg::render_scanlines_aa(rasterizer, scanline, rb_pre,
                                             sa, sg);
                }
            }

        }
    }
}
}// namespace mapnik
