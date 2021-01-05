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

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_RASTER_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_RASTER_SYMBOLIZER_HPP

// mapnik
#include <mapnik/image_util.hpp>
#include <mapnik/warp.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/feature.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_gray.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#pragma GCC diagnostic pop

namespace mapnik {

namespace detail {

template <typename F>
struct image_dispatcher
{
    using composite_function = F;
    image_dispatcher(int start_x, int start_y,
                          int width, int height,
                          double scale_x, double scale_y,
                          double offset_x, double offset_y,
                          scaling_method_e method, double filter_factor,
                          double opacity, composite_mode_e comp_op,
                          raster_symbolizer const& sym, feature_impl const& feature,
                          F & composite, boost::optional<double> const& nodata, bool need_scaling)
        : start_x_(start_x),
          start_y_(start_y),
          width_(width),
          height_(height),
          scale_x_(scale_x),
          scale_y_(scale_y),
          offset_x_(offset_x),
          offset_y_(offset_y),
          method_(method),
          filter_factor_(filter_factor),
          opacity_(opacity),
          comp_op_(comp_op),
          sym_(sym),
          feature_(feature),
          composite_(composite),
          nodata_(nodata),
          need_scaling_(need_scaling) {}

    void operator() (image_null const&) const {}  //no-op
    void operator() (image_rgba8 const& data_in) const
    {
        if (need_scaling_)
        {
            image_rgba8 data_out(width_, height_, true, true);
            scale_image_agg(data_out, data_in,  method_, scale_x_, scale_y_, offset_x_, offset_y_, filter_factor_, nodata_);
            composite_(data_out, comp_op_, opacity_, start_x_, start_y_);
        }
        else
        {
            composite_(data_in, comp_op_, opacity_, start_x_, start_y_);
        }
    }

    template <typename T>
    void operator() (T const& data_in) const
    {
        using image_type = T;
        image_rgba8 dst(width_, height_);
        raster_colorizer_ptr colorizer = get<raster_colorizer_ptr>(sym_, keys::colorizer);
        if (need_scaling_)
        {
            image_type data_out(width_, height_);
            scale_image_agg(data_out, data_in,  method_, scale_x_, scale_y_, offset_x_, offset_y_, filter_factor_, nodata_);
            if (colorizer) colorizer->colorize(dst, data_out, nodata_, feature_);
        }
        else
        {
            if (colorizer) colorizer->colorize(dst, data_in, nodata_, feature_);
        }
        premultiply_alpha(dst);
        composite_(dst, comp_op_, opacity_, start_x_, start_y_);
    }
private:
    int start_x_;
    int start_y_;
    int width_;
    int height_;
    double scale_x_;
    double scale_y_;
    double offset_x_;
    double offset_y_;
    scaling_method_e method_;
    double filter_factor_;
    double opacity_;
    composite_mode_e comp_op_;
    raster_symbolizer const& sym_;
    feature_impl const& feature_;
    composite_function & composite_;
    boost::optional<double> const& nodata_;
    bool need_scaling_;
};

template <typename F>
struct image_warp_dispatcher
{
    using composite_function = F;
    image_warp_dispatcher(proj_transform const& prj_trans,
                               int start_x, int start_y, int width, int height,
                               box2d<double> const& target_ext, box2d<double> const& source_ext,
                               double offset_x, double offset_y, unsigned mesh_size, scaling_method_e scaling_method,
                               double filter_factor, double opacity, composite_mode_e comp_op,
                               raster_symbolizer const& sym, feature_impl const& feature, F & composite, boost::optional<double> const& nodata)
        : prj_trans_(prj_trans),
        start_x_(start_x),
        start_y_(start_y),
        width_(width),
        height_(height),
        target_ext_(target_ext),
        source_ext_(source_ext),
        offset_x_(offset_x),
        offset_y_(offset_y),
        mesh_size_(mesh_size),
        scaling_method_(scaling_method),
        filter_factor_(filter_factor),
        opacity_(opacity),
        comp_op_(comp_op),
        sym_(sym),
        feature_(feature),
        composite_(composite),
        nodata_(nodata) {}

    void operator() (image_null const&) const {} //no-op

    void operator() (image_rgba8 const& data_in) const
    {
        image_rgba8 data_out(width_, height_, true, true);
        warp_image(data_out, data_in, prj_trans_, target_ext_, source_ext_, offset_x_, offset_y_, mesh_size_, scaling_method_, filter_factor_, nodata_);
        composite_(data_out, comp_op_, opacity_, start_x_, start_y_);
    }

    template <typename T>
    void operator() (T const& data_in) const
    {
        using image_type = T;
        image_type data_out(width_, height_);
        if (nodata_) data_out.set(*nodata_);
        warp_image(data_out, data_in, prj_trans_, target_ext_, source_ext_, offset_x_, offset_y_, mesh_size_, scaling_method_, filter_factor_, nodata_);
        image_rgba8 dst(width_, height_);
        raster_colorizer_ptr colorizer = get<raster_colorizer_ptr>(sym_, keys::colorizer);
        if (colorizer) colorizer->colorize(dst, data_out, nodata_, feature_);
        premultiply_alpha(dst);
        composite_(dst, comp_op_, opacity_, start_x_, start_y_);
    }
private:
    proj_transform const& prj_trans_;
    int start_x_;
    int start_y_;
    int width_;
    int height_;
    box2d<double> const& target_ext_;
    box2d<double> const& source_ext_;
    double offset_x_;
    double offset_y_;
    unsigned mesh_size_;
    scaling_method_e scaling_method_;
    double filter_factor_;
    double opacity_;
    composite_mode_e comp_op_;
    raster_symbolizer const& sym_;
    feature_impl const& feature_;
    composite_function & composite_;
    boost::optional<double> const& nodata_;
};

}

template <typename F>
void render_raster_symbolizer(raster_symbolizer const& sym,
                              mapnik::feature_impl& feature,
                              proj_transform const& prj_trans,
                              renderer_common& common,
                              F composite)
{
    raster_ptr const& source = feature.get_raster();
    if (source)
    {
        box2d<double> target_ext = box2d<double>(source->ext_);
        box2d<double> target_query_ext = box2d<double>(source->query_ext_);
        if (!prj_trans.equal())
        {
            prj_trans.backward(target_ext, PROJ_ENVELOPE_POINTS);
            prj_trans.backward(target_query_ext, PROJ_ENVELOPE_POINTS);
        }
        box2d<double> ext = common.t_.forward(target_ext);
        box2d<double> query_ext = common.t_.forward(target_query_ext);
        int start_x = static_cast<int>(std::floor(query_ext.minx()+.5));
        int start_y = static_cast<int>(std::floor(query_ext.miny()+.5));
        int end_x = static_cast<int>(std::floor(query_ext.maxx()+.5));
        int end_y = static_cast<int>(std::floor(query_ext.maxy()+.5));
        int raster_width = end_x - start_x;
        int raster_height = end_y - start_y;
        if (raster_width > 0 && raster_height > 0)
        {
            scaling_method_e scaling_method = get<scaling_method_e>(sym, keys::scaling, feature, common.vars_, SCALING_NEAR);
            composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common.vars_, src_over);
            double opacity = get<double>(sym,keys::opacity,feature, common.vars_, 1.0);
            // only premultiply rgba8 images
            if (source->data_.is<image_rgba8>())
            {
                auto is_premultiplied = get_optional<bool>(sym, keys::premultiplied, feature, common.vars_);
                if (is_premultiplied && *is_premultiplied)
                {
                    mapnik::set_premultiplied_alpha(source->data_, true);
                }
                mapnik::premultiply_alpha(source->data_);
            }

            if (!prj_trans.equal())
            {
                // This path does not currently work and is still being figured out. 
                double offset_x = query_ext.minx() - start_x;
                double offset_y = query_ext.miny() - start_y;
                unsigned mesh_size = static_cast<unsigned>(get<value_integer>(sym,keys::mesh_size,feature, common.vars_, 16));
                detail::image_warp_dispatcher<F> dispatcher(prj_trans, start_x, start_y, raster_width, raster_height,
                                                                 target_query_ext, source->ext_, offset_x, offset_y, mesh_size,
                                                                 scaling_method, source->get_filter_factor(),
                                                                 opacity, comp_op, sym, feature, composite, source->nodata());
                util::apply_visitor(dispatcher, source->data_);
            }
            else
            {
                double offset_x = query_ext.minx() - ext.minx();
                double offset_y = query_ext.miny() - ext.miny();
                double image_ratio_x = ext.width() / source->data_.width();
                double image_ratio_y = ext.height() / source->data_.height();
                double eps = 1e-5;
                bool scale = (std::fabs(image_ratio_x - 1.0) > eps) ||
                     (std::fabs(image_ratio_y - 1.0) > eps) ||
                     (std::abs(start_x) > eps) ||
                     (std::abs(start_y) > eps);
                detail::image_dispatcher<F> dispatcher(start_x, start_y, raster_width, raster_height,
                                                            image_ratio_x, image_ratio_y,
                                                            offset_x, offset_y,
                                                            scaling_method, source->get_filter_factor(),
                                                            opacity, comp_op, sym, feature, composite, source->nodata(), scale);
                util::apply_visitor(dispatcher, source->data_);
            }
        }
    }
}

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_RASTER_SYMBOLIZER_HPP
