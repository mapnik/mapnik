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
#include <mapnik/debug.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg/render_polygon_pattern.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
// for polygon_pattern_symbolizer
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"
#include "agg_conv_clip_polygon.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

template <typename buffer_type>
struct agg_renderer_process_visitor_p
{
    agg_renderer_process_visitor_p(renderer_common & common,
                                   buffer_type & current_buffer,
                                   std::unique_ptr<rasterizer> const& ras_ptr,
                                   gamma_method_enum & gamma_method,
                                   double & gamma,
                                   polygon_pattern_symbolizer const& sym,
                                   mapnik::feature_impl & feature,
                                   proj_transform const& prj_trans)
    : common_(common),
        current_buffer_(current_buffer),
        ras_ptr_(ras_ptr),
        gamma_method_(gamma_method),
        gamma_(gamma),
        sym_(sym),
        feature_(feature),
        prj_trans_(prj_trans) {}

    void operator() (marker_null const&) const {}

    void operator() (marker_svg const& marker) const
    {
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform) evaluate_transform(image_tr, feature_, common_.vars_, *image_transform, common_.scale_factor_);
        mapnik::box2d<double> const& bbox_image = marker.get_data()->bounding_box() * image_tr;
        mapnik::image_rgba8 image(bbox_image.width(), bbox_image.height());
        render_pattern<buffer_type>(marker, image_tr, 1.0, image);
        render(image);
    }

    void operator() (marker_rgba8 const& marker) const
    {
        render(marker.get_data());
    }

private:
    void render(mapnik::image_rgba8 const& image) const
    {
        agg::rendering_buffer buf(current_buffer_.bytes(), current_buffer_.width(),
                                  current_buffer_.height(), current_buffer_.row_size());
        ras_ptr_->reset();
        value_double gamma = get<value_double, keys::gamma>(sym_, feature_, common_.vars_);
        gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym_, feature_, common_.vars_);
        if (gamma != gamma_ || gamma_method != gamma_method_)
        {
            set_gamma_method(ras_ptr_, gamma, gamma_method);
            gamma_method_ = gamma_method;
            gamma_ = gamma;
        }

        using vertex_converter_type = vertex_converter<clip_poly_tag,
                                                       transform_tag,
                                                       affine_transform_tag,
                                                       simplify_tag,
                                                       smooth_tag>;
        using pattern_type = agg_polygon_pattern<vertex_converter_type>;

        pattern_type pattern(image, common_, sym_, feature_, prj_trans_);

        pattern_type::pixfmt_type pixf(buf);
        pixf.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym_, feature_, common_.vars_)));
        pattern_type::renderer_base renb(pixf);

        unsigned w = image.width();
        unsigned h = image.height();
        agg::rendering_buffer pattern_rbuf((agg::int8u*)image.bytes(),w,h,w*4);
        agg::pixfmt_rgba32_pre pixf_pattern(pattern_rbuf);
        pattern_type::img_source_type img_src(pixf_pattern);

        if (prj_trans_.equal() && pattern.clip_) pattern.converter_.set<clip_poly_tag>();

        ras_ptr_->filling_rule(agg::fill_even_odd);

        pattern.render(renb, *ras_ptr_);
    }

    renderer_common & common_;
    buffer_type & current_buffer_;
    std::unique_ptr<rasterizer> const& ras_ptr_;
    gamma_method_enum & gamma_method_;
    double & gamma_;
    polygon_pattern_symbolizer const& sym_;
    mapnik::feature_impl & feature_;
    proj_transform const& prj_trans_;
};

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(polygon_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    if (filename.empty()) return;
    std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);
    agg_renderer_process_visitor_p<buffer_type> visitor(common_,
                                                        buffers_.top().get(),
                                                        ras_ptr,
                                                        gamma_method_,
                                                        gamma_,
                                                        sym,
                                                        feature,
                                                        prj_trans);
    util::apply_visitor(visitor, *marker);
}


template void agg_renderer<image_rgba8>::process(polygon_pattern_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}
