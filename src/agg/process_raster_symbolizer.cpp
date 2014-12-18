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
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/raster_symbolizer.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/warp.hpp>
#include <mapnik/config.hpp>

// stl
#include <cmath>

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"

#define MAX_SIZE 1024*1024


namespace mapnik {


template <typename T>
void agg_renderer<T>::process(raster_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)
{
    raster_ptr const& source = feature.get_raster();
    if (source)
    {
        // If there's a colorizer defined, use it to color the raster in-place
        raster_colorizer_ptr colorizer = sym.get_colorizer();
        if (colorizer)
            colorizer->colorize(source,feature);

        box2d<double> target_ext = box2d<double>(source->ext_);
        prj_trans.backward(target_ext, PROJ_ENVELOPE_POINTS);
        box2d<double> ext = t_.forward(target_ext);
        int start_x = static_cast<int>(std::floor(ext.minx()+.5));
        int start_y = static_cast<int>(std::floor(ext.miny()+.5));
        int end_x = static_cast<int>(std::floor(ext.maxx()+.5));
        int end_y = static_cast<int>(std::floor(ext.maxy()+.5));
        int raster_width = end_x - start_x;
        int raster_height = end_y - start_y;
        int raster_size = raster_width * raster_height;

        if (raster_width > 0 && raster_height > 0  && raster_size > 0 && raster_size <= MAX_SIZE)
        {
            scaling_method_e scaling_method = sym.get_scaling_method();
            double filter_radius = sym.calculate_filter_factor();
            bool premultiply_source = !source->premultiplied_alpha_;
            boost::optional<bool> is_premultiplied = sym.premultiplied();
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
                raster target(target_ext, raster_width, raster_height);
                reproject_and_scale_raster(target, *source, prj_trans,
                                 offset_x, offset_y,
                                 sym.get_mesh_size(),
                                 filter_radius,
                                 scaling_method);
                composite(current_buffer_->data(), target.data_,
                          sym.comp_op(), sym.get_opacity(),
                          start_x, start_y, false);
            }
            else
            {
                double image_ratio_x = ext.width() / source->data_.width();
                double image_ratio_y = ext.height() / source->data_.height();
                double eps = 1e-5;
                if ( (std::fabs(image_ratio_x - 1.0) <= eps) &&
                     (std::fabs(image_ratio_y - 1.0) <= eps) &&
                     (std::abs(start_x) <= eps) &&
                     (std::abs(start_y) <= eps) )
                {
                    composite(current_buffer_->data(), source->data_,
                              sym.comp_op(), sym.get_opacity(),
                              start_x, start_y, false);
                }
                else
                {
                    raster target(target_ext, raster_width, raster_height);
                    if (scaling_method == SCALING_BILINEAR8)
                    {
                        scale_image_bilinear8<image_data_32>(target.data_,
                                                             source->data_,
                                                             0.0,
                                                             0.0);
                    }
                    else
                    {
                        scale_image_agg<image_data_32>(target.data_,
                                                       source->data_,
                                                       scaling_method,
                                                       image_ratio_x,
                                                       image_ratio_y,
                                                       0.0,
                                                       0.0,
                                                       filter_radius);
                    }
                    composite(current_buffer_->data(), target.data_,
                              sym.comp_op(), sym.get_opacity(),
                              start_x, start_y, false);

                }
            }
        }
    }
}

template void agg_renderer<image_32>::process(raster_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
