/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_RASTER_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_RASTER_SYMBOLIZER_HPP

// agg
#include "agg_conv_clip_polyline.h"
#include "agg_conv_clip_polygon.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"

namespace mapnik {

template <typename F>
void render_raster_symbolizer(raster_symbolizer const &sym,
                              mapnik::feature_impl &feature,
                              proj_transform const &prj_trans,
                              renderer_common &common,
                              F composite)
{
    raster_ptr const& source = feature.get_raster();
    if (source)
    {
        // If there's a colorizer defined, use it to color the raster in-place
        raster_colorizer_ptr colorizer = get<raster_colorizer_ptr>(sym, keys::colorizer);
        if (colorizer)
            colorizer->colorize(source,feature);

        box2d<double> target_ext = box2d<double>(source->ext_);
        prj_trans.backward(target_ext, PROJ_ENVELOPE_POINTS);
        box2d<double> ext = common.t_.forward(target_ext);
        int start_x = static_cast<int>(std::floor(ext.minx()+.5));
        int start_y = static_cast<int>(std::floor(ext.miny()+.5));
        int end_x = static_cast<int>(std::floor(ext.maxx()+.5));
        int end_y = static_cast<int>(std::floor(ext.maxy()+.5));
        int raster_width = end_x - start_x;
        int raster_height = end_y - start_y;
        if (raster_width > 0 && raster_height > 0)
        {
            raster target(target_ext, raster_width, raster_height, source->get_filter_factor());
            scaling_method_e scaling_method = get<scaling_method_e>(sym, keys::scaling, feature, SCALING_NEAR);
            composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, src_over);

            double opacity = get<double>(sym,keys::opacity,feature, 1.0);
            bool premultiply_source = !source->premultiplied_alpha_;
            auto is_premultiplied = get_optional<bool>(sym, keys::premultiplied);
            if (is_premultiplied)
            {
                if (*is_premultiplied) premultiply_source = false;
                else premultiply_source = true;
            }
            if (premultiply_source)
            {
                agg::rendering_buffer buffer(source->data_.getBytes(),
                                             source->data_.width(),
                                             source->data_.height(),
                                             source->data_.width() * 4);
                agg::pixfmt_rgba32 pixf(buffer);
                pixf.premultiply();
            }
            if (!prj_trans.equal())
            {
                double offset_x = ext.minx() - start_x;
                double offset_y = ext.miny() - start_y;
                unsigned mesh_size = static_cast<unsigned>(get<value_integer>(sym,keys::mesh_size,feature, 16));
                reproject_and_scale_raster(target,
                                           *source,
                                           prj_trans,
                                           offset_x,
                                           offset_y,
                                           mesh_size,
                                           scaling_method);
            }
            else
            {
                if (scaling_method == SCALING_BILINEAR8)
                {
                    scale_image_bilinear8<image_data_32>(target.data_,
                                                         source->data_,
                                                         0.0,
                                                         0.0);
                }
                else
                {
                    double image_ratio_x = ext.width() / source->data_.width();
                    double image_ratio_y = ext.height() / source->data_.height();
                    scale_image_agg<image_data_32>(target.data_,
                                                   source->data_,
                                                   scaling_method,
                                                   image_ratio_x,
                                                   image_ratio_y,
                                                   0.0,
                                                   0.0,
                                                   source->get_filter_factor());
                }
            }
            composite(target, comp_op, opacity, start_x, start_y);
        }
    }
}

} // namespace mapnik

#endif /* MAPNIK_RENDERER_COMMON_PROCESS_RASTER_SYMBOLIZER_HPP */
