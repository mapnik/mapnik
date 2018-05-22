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
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_render_marker.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/renderer_common/render_markers_symbolizer.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_path_storage.h"
#include "agg_conv_transform.h"
#pragma GCC diagnostic pop

namespace mapnik {

namespace detail {

template <typename SvgRenderer, typename BufferType, typename RasterizerType>
struct agg_markers_renderer_context : markers_renderer_context
{
    using renderer_base = typename SvgRenderer::renderer_base;
    using vertex_source_type = typename SvgRenderer::vertex_source_type;
    using pixfmt_type = typename renderer_base::pixfmt_type;

    agg_markers_renderer_context(markers_symbolizer const& sym,
                                 feature_impl const& feature,
                                 attributes const& vars,
                                 BufferType & buf,
                                 RasterizerType & ras)
      : buf_(buf),
        pixf_(buf_),
        renb_(pixf_),
        ras_(ras),
        sym_cacheable(sym.cacheable == markers_symbolizer::cache_status::CACHEABLE)
    {
        auto comp_op = get<composite_mode_e, keys::comp_op>(sym, feature, vars);
        pixf_.comp_op(static_cast<agg::comp_op_e>(comp_op));
    }

    virtual void render_marker(svg_path_ptr const& src,
                               svg_path_adapter & path,
                               svg_attribute_type const& attrs,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr)
    {
        // Check that the symbolizer is cacheable (doesn't use expressions)
        // and the transform doesn't use any scaling/shearing.
        bool mark_cacheable = (std::fabs(1.0 - marker_tr.sx)  < agg::affine_epsilon)
                           && (std::fabs(0.0 - marker_tr.shy) < agg::affine_epsilon)
                           && (std::fabs(0.0 - marker_tr.shx) < agg::affine_epsilon)
                           && (std::fabs(1.0 - marker_tr.sy)  < agg::affine_epsilon);
        if (!sym_cacheable || !mark_cacheable)
        {
            // Fallback to non-cached rendering path
            SvgRenderer svg_renderer(path, attrs);
            render_vector_marker(svg_renderer, ras_, renb_, src->bounding_box(),
                                 marker_tr, params.opacity, params.snap_to_pixels);
            return;
        }

        for (auto & attr : attrs)
        {
            using mapnik::svg::path_attributes;
            auto & cattr = const_cast<path_attributes &>(attr);
            constexpr int sampling_rate = path_attributes::sampling_rate;
            // Calculate canvas offsets
            double margin = 0.0;
            if (cattr.stroke_flag || cattr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
            {
                margin = std::abs(cattr.stroke_width);
            }
            double x0 = std::floor(src->bounding_box().minx() - margin);
            double y0 = std::floor(src->bounding_box().miny() - margin);

            // Now calculate subpixel offset and sample index
            double dx = marker_tr.tx - std::floor(marker_tr.tx);
            double dy = marker_tr.ty - std::floor(marker_tr.ty);

            double sample_x = std::floor(dx * sampling_rate);
            double sample_y = std::floor(dy * sampling_rate);

            const int sample_idx = static_cast<int>(sample_y) *
                                   sampling_rate + static_cast<int>(sample_x);


            if (cattr.cached_images.empty())
            {
                cattr.cached_images.resize(sampling_rate * sampling_rate);
            }

            path_attributes::cache_line & cl = cattr.cached_images[sample_idx];
            if (!cl.set)
            {
                std::shared_ptr<image_rgba8> fill_img = nullptr;
                std::shared_ptr<image_rgba8> stroke_img = nullptr;

                // Calculate canvas size
                int width  = static_cast<int>(std::ceil(src->bounding_box().width()  + 2.0 * margin)) + 2;
                int height = static_cast<int>(std::ceil(src->bounding_box().height() + 2.0 * margin)) + 2;

                // Reset clip box
                ras_.clip_box(0, 0, width, height);

                // Build local transformation matrix by resetting translation coordinates
                agg::trans_affine marker_tr_copy(marker_tr);
                marker_tr_copy.tx = dx - x0;
                marker_tr_copy.ty = dy - y0;

                // Create fill image
                if (cattr.fill_flag || cattr.fill_gradient.get_gradient_type() != NO_GRADIENT)
                {
                    fill_img = std::make_shared<image_rgba8>(width, height, true);

                    agg::rendering_buffer buf(fill_img->bytes(), fill_img->width(), fill_img->height(), fill_img->row_size());
                    pixfmt_type pixf(buf);
                    renderer_base renb(pixf);

                    auto cattr_copy = cattr;
                    cattr_copy.stroke_flag = false;
                    cattr_copy.stroke_gradient.set_gradient_type(NO_GRADIENT);
                    svg_attribute_type attrs_copy;
                    attrs_copy.push_back(cattr_copy);

                    SvgRenderer svg_renderer(path, attrs_copy);
                    render_vector_marker(svg_renderer, ras_, renb, src->bounding_box(),
                                         marker_tr_copy, 1.0, params.snap_to_pixels);

                    if (std::all_of(fill_img->begin(), fill_img->end(), [](uint32_t val) { return val == 0; }))
                    {
                        fill_img.reset(); // optimization, can ignore image
                    }
                }

                // Create stroke image
                if (cattr.stroke_flag || cattr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
                {
                    stroke_img = std::make_shared<image_rgba8>(width, height, true);

                    agg::rendering_buffer buf(stroke_img->bytes(), stroke_img->width(), stroke_img->height(), stroke_img->row_size());
                    pixfmt_type pixf(buf);
                    renderer_base renb(pixf);

                    auto cattr_copy = cattr;
                    cattr_copy.fill_flag = false;
                    cattr_copy.fill_gradient.set_gradient_type(NO_GRADIENT);
                    svg_attribute_type attrs_copy;
                    attrs_copy.push_back(cattr_copy);

                    SvgRenderer svg_renderer(path, attrs_copy);
                    render_vector_marker(svg_renderer, ras_, renb, src->bounding_box(),
                                         marker_tr_copy, 1.0, params.snap_to_pixels);

                    if (std::all_of(stroke_img->begin(), stroke_img->end(), [](uint32_t val) { return val == 0; }))
                    {
                        stroke_img.reset(); // optimization, can ignore image
                    }
                }

                // Update cache with the new images
                cl.set = true;
                cl.fill_img = fill_img;
                cl.stroke_img = stroke_img;

                // Restore clip box
                ras_.clip_box(0, 0, pixf_.width(), pixf_.height());
            }

            // Set up blitting transformation. We will add a small offset due to sampling
            agg::trans_affine marker_tr_copy(marker_tr);
            marker_tr_copy.translate(x0 - dx, y0 - dy);

            // Blit stroke and fill images
            if (cl.fill_img)
            {
                render_raster_marker(renb_, ras_, *cl.fill_img, marker_tr_copy,
                                    params.opacity, params.scale_factor, params.snap_to_pixels);
            }
            if (cl.stroke_img)
            {
                render_raster_marker(renb_, ras_, *cl.stroke_img, marker_tr_copy,
                                    params.opacity, params.scale_factor, params.snap_to_pixels);
            }
        }
    }


    virtual void render_marker(image_rgba8 const& src,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr)
    {
        // In the long term this should be a visitor pattern based on the type of
        // render src provided that converts the destination pixel type required.
        render_raster_marker(renb_, ras_, src, marker_tr, params.opacity,
                             params.scale_factor, params.snap_to_pixels);
    }

private:
    BufferType & buf_;
    pixfmt_type pixf_;
    renderer_base renb_;
    RasterizerType & ras_;
    bool sym_cacheable;
};

} // namespace detail

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(markers_symbolizer const& sym,
                              feature_impl & feature,
                              proj_transform const& prj_trans)
{
    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
    using buf_type = agg::rendering_buffer;
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, buf_type>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;
    using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;
    using svg_renderer_type = svg::renderer_agg<svg_path_adapter,
                                                svg_attribute_type,
                                                renderer_type,
                                                pixfmt_comp_type>;
    ras_ptr->reset();

    double gamma = get<value_double, keys::gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym, feature, common_.vars_);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    buffer_type & current_buffer = buffers_.top().get();
    buf_type render_buffer(current_buffer.bytes(), current_buffer.width(), current_buffer.height(), current_buffer.row_size());
    box2d<double> clip_box = clipping_extent(common_);

    using renderer_context_type = detail::agg_markers_renderer_context<svg_renderer_type,
                                                              buf_type,
                                                              rasterizer>;
    renderer_context_type renderer_context(sym, feature, common_.vars_, render_buffer, *ras_ptr);

    render_markers_symbolizer(
        sym, feature, prj_trans, common_, clip_box, renderer_context);
}

template void agg_renderer<image_rgba8>::process(markers_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
} // namespace mapnik
