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
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/map.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_filter.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/safe_cast.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_scanline_u.h"
#include "agg_image_filters.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <algorithm>
#include <cmath>
#include <vector>

namespace mapnik {

namespace {

// The columns from the first to the last nonzero pixel of a row.
struct row_span
{
    int y;
    int x0;
    int x1;
};

// Collects the span of every row that has a nonzero pixel.
// The spans of non-transparent pixels in the columns x0..x1 of the rows y0..y1 (half-open; all
// pixels outside them must be transparent).
void nonzero_spans(image_rgba8 const& image, std::vector<row_span>& spans, int x0, int y0, int x1, int y1)
{
    spans.clear();
    for (int y = y0; y < y1; ++y)
    {
        image_rgba8::pixel_type const* row = image.get_row(y);
        int row_minx = x0;
        while (row_minx + 8 <= x1 &&
               (row[row_minx] | row[row_minx + 1] | row[row_minx + 2] | row[row_minx + 3] | row[row_minx + 4] |
                row[row_minx + 5] | row[row_minx + 6] | row[row_minx + 7]) == 0)
        {
            row_minx += 8;
        }
        while (row_minx < x1 && row[row_minx] == 0)
        {
            ++row_minx;
        }
        if (row_minx == x1)
        {
            continue;
        }
        int row_maxx = x1;
        while (row_maxx - 8 >= x0 && (row[row_maxx - 1] | row[row_maxx - 2] | row[row_maxx - 3] | row[row_maxx - 4] |
                                      row[row_maxx - 5] | row[row_maxx - 6] | row[row_maxx - 7] | row[row_maxx - 8]) == 0)
        {
            row_maxx -= 8;
        }
        while (row[row_maxx - 1] == 0)
        {
            --row_maxx;
        }
        spans.push_back({y, row_minx, row_maxx - 1});
    }
}

// The rows the rasterizer swept since it last forgot, each from its first to its last span,
// within a width x height buffer: all pixels it rendered lie within them.
void rendered_spans(rasterizer const& ras, std::vector<row_span>& spans, int width, int height)
{
    spans.clear();
    int const y1 = std::min(ras.rendered_y2(), height - 1);
    for (int y = std::max(ras.rendered_y1(), 0); y <= y1; ++y)
    {
        int x0, x1;
        if (ras.rendered_row(y, x0, x1))
        {
            x0 = std::max(x0, 0);
            x1 = std::min(x1, width - 1);
            if (x0 <= x1)
            {
                spans.push_back({y, x0, x1});
            }
        }
    }
}

// Composites only the given spans of src, merging runs of rows with equal spans.
// Pixels outside them are transparent, which the callers' modes leave unchanged.
void composite_spans(image_rgba8& dst,
                     image_rgba8 const& src,
                     composite_mode_e mode,
                     std::vector<row_span> const& spans,
                     float opacity,
                     int dx,
                     int dy)
{
    std::size_t i = 0;
    while (i < spans.size())
    {
        std::size_t j = i + 1;
        while (j < spans.size() && spans[j].y == spans[j - 1].y + 1 && spans[j].x0 == spans[i].x0 &&
               spans[j].x1 == spans[i].x1)
        {
            ++j;
        }
        composite(dst, src, mode, box2d<int>(spans[i].x0, spans[i].y, spans[i].x1, spans[j - 1].y), opacity, dx, dy);
        i = j;
    }
}

void clear_spans(image_rgba8& image, std::vector<row_span> const& spans)
{
    for (row_span const& span : spans)
    {
        std::fill_n(image.get_row(span.y, span.x0), span.x1 - span.x0 + 1, 0);
    }
}

} // namespace

template<typename T0, typename T1>
agg_renderer<T0, T1>::agg_renderer(Map const& m, T0& pixmap, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<agg_renderer>(m, scale_factor),
      buffers_(),
      internal_buffers_(m.width(), m.height()),
      inflated_buffer_(),
      ras_ptr(std::make_unique<rasterizer>()),
      gamma_method_(gamma_method_enum::GAMMA_POWER),
      gamma_(1.0),
      common_(m, attributes(), offset_x, offset_y, m.width(), m.height(), scale_factor)
{
    setup(m, pixmap);
}

template<typename T0, typename T1>
agg_renderer<T0, T1>::agg_renderer(Map const& m,
                                   request const& req,
                                   attributes const& vars,
                                   T0& pixmap,
                                   double scale_factor,
                                   unsigned offset_x,
                                   unsigned offset_y)
    : feature_style_processor<agg_renderer>(m, scale_factor),
      buffers_(),
      internal_buffers_(req.width(), req.height()),
      inflated_buffer_(),
      ras_ptr(std::make_unique<rasterizer>()),
      gamma_method_(gamma_method_enum::GAMMA_POWER),
      gamma_(1.0),
      common_(m, req, vars, offset_x, offset_y, req.width(), req.height(), scale_factor)
{
    setup(m, pixmap);
}

template<typename T0, typename T1>
agg_renderer<T0, T1>::agg_renderer(Map const& m,
                                   T0& pixmap,
                                   std::shared_ptr<T1> detector,
                                   double scale_factor,
                                   unsigned offset_x,
                                   unsigned offset_y)
    : feature_style_processor<agg_renderer>(m, scale_factor),
      buffers_(),
      internal_buffers_(m.width(), m.height()),
      inflated_buffer_(),
      ras_ptr(std::make_unique<rasterizer>()),
      gamma_method_(gamma_method_enum::GAMMA_POWER),
      gamma_(1.0),
      common_(m, attributes(), offset_x, offset_y, m.width(), m.height(), scale_factor, detector)
{
    setup(m, pixmap);
}

template<typename buffer_type>
struct setup_agg_bg_visitor
{
    setup_agg_bg_visitor(buffer_type& pixmap, renderer_common const& common, composite_mode_e mode, double opacity)
        : pixmap_(pixmap),
          common_(common),
          mode_(mode),
          opacity_(opacity)
    {}

    void operator()(marker_null const&) const {}

    void operator()(marker_svg const&) const {}

    void operator()(marker_rgba8 const& marker) const
    {
        mapnik::image_rgba8 const& bg_image = marker.get_data();
        std::size_t w = bg_image.width();
        std::size_t h = bg_image.height();
        if (w > 0 && h > 0)
        {
            // repeat background-image both vertically and horizontally
            std::size_t x_steps = static_cast<std::size_t>(std::ceil(common_.width_ / double(w)));
            std::size_t y_steps = static_cast<std::size_t>(std::ceil(common_.height_ / double(h)));
            for (std::size_t x = 0; x < x_steps; ++x)
            {
                for (std::size_t y = 0; y < y_steps; ++y)
                {
                    composite(pixmap_, bg_image, mode_, opacity_, x * w, y * h);
                }
            }
        }
    }

  private:
    buffer_type& pixmap_;
    renderer_common const& common_;
    composite_mode_e mode_;
    double opacity_;
};

template<typename T0, typename T1>
void agg_renderer<T0, T1>::setup(Map const& m, buffer_type& pixmap)
{
    buffers_.emplace(pixmap);

    mapnik::set_premultiplied_alpha(pixmap, true);
    auto&& bg = m.background();
    if (bg)
    {
        if (bg->alpha() < 255)
        {
            mapnik::color bg_color = *bg;
            bg_color.premultiply();
            mapnik::fill(pixmap, bg_color);
        }
        else
        {
            mapnik::color bg_color = *bg;
            bg_color.set_premultiplied(true);
            mapnik::fill(pixmap, bg_color);
        }
    }

    auto&& image_filename = m.background_image();
    if (image_filename)
    {
        // NOTE: marker_cache returns premultiplied image, if needed
        std::shared_ptr<mapnik::marker const> bg_marker = mapnik::marker_cache::instance().find(*image_filename, true);
        setup_agg_bg_visitor<buffer_type> visitor(pixmap,
                                                  common_,
                                                  m.background_image_comp_op(),
                                                  m.background_image_opacity());
        util::apply_visitor(visitor, *bg_marker);
    }
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Scale=" << m.scale();
}

template<typename T0, typename T1>
agg_renderer<T0, T1>::~agg_renderer()
{}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::start_map_processing(Map const& map)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Start map processing bbox=" << map.get_current_extent();
    ras_ptr->clip_box(0, 0, common_.width_, common_.height_);
}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::end_map_processing(Map const& map)
{
    mapnik::demultiply_alpha(buffers_.top().get());
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: End map processing";
}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Start processing layer=" << lay.name();
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: -- datasource=" << lay.datasource().get();
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: -- query_extent=" << query_extent;

    if (lay.clear_label_cache())
    {
        common_.detector_->clear();
    }

    common_.query_extent_ = query_extent;
    auto&& maximum_extent = lay.maximum_extent();
    if (maximum_extent)
    {
        common_.query_extent_.clip(*maximum_extent);
    }

    if (lay.comp_op() || lay.get_opacity() < 1.0)
    {
        buffers_.emplace(internal_buffers_.push());
        // composited whole
        internal_buffers_.clean_top();
        set_premultiplied_alpha(buffers_.top().get(), true);
    }
    else
    {
        buffers_.emplace(buffers_.top().get());
    }
}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::end_layer_processing(layer const& lyr)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: End layer processing";

    buffer_type& current_buffer = buffers_.top().get();
    buffers_.pop();
    buffer_type& previous_buffer = buffers_.top().get();

    if (&current_buffer != &previous_buffer)
    {
        composite_mode_e comp_op = lyr.comp_op() ? *lyr.comp_op() : src_over;
        composite(previous_buffer, current_buffer, comp_op, lyr.get_opacity(), 0, 0);
        // A parent layer may contain only pixels composited from its children.
        previous_buffer.painted(previous_buffer.painted() || current_buffer.painted());
        internal_buffers_.pop();
    }
}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::start_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Start processing style";

    drew_otherwise_ = false;
    ras_ptr->forget_rendered();
    composited_ = st.comp_op() || st.image_filters().size() > 0 || st.get_opacity() < 1;
    // what the rasterizer renders and how the style's symbolizers draw matter only for a buffer
    // that is composited (see end_style_processing)
    ras_ptr->track_rendered(composited_);
    if (composited_)
    {
        if (st.image_filters_inflate())
        {
            int radius = 0;
            mapnik::filter::filter_radius_visitor visitor(radius);
            for (mapnik::filter::filter_type const& filter_tag : st.image_filters())
            {
                util::apply_visitor(visitor, filter_tag);
            }
            radius *= common_.scale_factor_;
            if (radius > common_.t_.offset())
            {
                common_.t_.set_offset(radius);
            }
            int offset = common_.t_.offset();
            unsigned target_width = common_.width_ + (offset * 2);
            unsigned target_height = common_.height_ + (offset * 2);
            ras_ptr->clip_box(-int(offset * 2), -int(offset * 2), target_width, target_height);
            if (!inflated_buffer_ ||
                (inflated_buffer_->width() < target_width || inflated_buffer_->height() < target_height))
            {
                inflated_buffer_ = std::make_unique<buffer_type>(target_width, target_height);
            }
            else if (inflated_buffer_->painted())
            {
                mapnik::fill(*inflated_buffer_, 0); // fill with transparent colour
            }
            inflated_buffer_->painted(false);
            buffers_.emplace(*inflated_buffer_);
        }
        else
        {
            buffers_.emplace(internal_buffers_.push());
            common_.t_.set_offset(0);
            ras_ptr->clip_box(0, 0, common_.width_, common_.height_);
            if (st.image_filters().empty() && (!st.comp_op() || *st.comp_op() == src_over || *st.comp_op() == darken))
            {
                // Only the rows drawn on are composited (see end_style_processing), so the
                // buffer is cleared row by row as the rasterizer first sweeps each: a style
                // that draws some other way clears the rest first (drew_otherwise).
                buffer_type& buffer = buffers_.top().get();
                ras_ptr->clear_rows_lazily(buffer.bytes(),
                                           buffer.row_size(),
                                           internal_buffers_.top_clean(),
                                           buffer.height());
                lazy_rows_ = true;
            }
            else
            {
                internal_buffers_.clean_top();
            }
        }
        set_premultiplied_alpha(buffers_.top().get(), true);
    }
    else
    {
        common_.t_.set_offset(0);
        ras_ptr->clip_box(0, 0, common_.width_, common_.height_);
        buffers_.emplace(buffers_.top().get());
    }
}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::drew_otherwise()
{
    drew_otherwise_ = true;
    if (lazy_rows_)
    {
        lazy_rows_ = false;
        ras_ptr->clear_rows_eagerly();
        internal_buffers_.clean_top();
    }
}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::end_style_processing(feature_type_style const& st)
{
    lazy_rows_ = false;
    ras_ptr->clear_rows_eagerly();
    buffer_type& current_buffer = buffers_.top().get();
    buffers_.pop();
    buffer_type& previous_buffer = buffers_.top().get();
    if (&current_buffer != &previous_buffer)
    {
        bool const transparent_is_identity = !st.comp_op() || *st.comp_op() == src_over || *st.comp_op() == darken;
        bool const skip_empty = !current_buffer.painted() && st.image_filters().empty() && transparent_is_identity;
        if (!skip_empty)
        {
            bool blend_from = false;
            if (st.image_filters().size() > 0)
            {
                blend_from = true;
                mapnik::filter::filter_visitor<buffer_type> visitor(current_buffer, common_.scale_factor_);
                for (mapnik::filter::filter_type const& filter_tag : st.image_filters())
                {
                    util::apply_visitor(visitor, filter_tag);
                }
                mapnik::premultiply_alpha(current_buffer);
                // Filters can generate pixels even when no rule painted the buffer.
                current_buffer.painted(true);
            }
            std::vector<row_span> spans;
            // a buffer is clear when its style starts: spans can only be where something drew
            int const width = safe_cast<int>(current_buffer.width());
            int const height = safe_cast<int>(current_buffer.height());
            if (transparent_is_identity && (blend_from || drew_otherwise_))
            {
                nonzero_spans(current_buffer, spans, 0, 0, width, height);
            }
            else if (transparent_is_identity && ras_ptr->rendered())
            {
                // only the rasterizer drew: the rows it swept, which may hold transparent pixels
                // too (the modes above leave the destination as it is under those)
                rendered_spans(*ras_ptr, spans, width, height);
            }
            if (!spans.empty() && st.comp_op())
            {
                composite_spans(previous_buffer,
                                current_buffer,
                                *st.comp_op(),
                                spans,
                                st.get_opacity(),
                                -common_.t_.offset(),
                                -common_.t_.offset());
            }
            else if (!spans.empty() && (blend_from || st.get_opacity() < 1.0))
            {
                composite_spans(previous_buffer,
                                current_buffer,
                                src_over,
                                spans,
                                st.get_opacity(),
                                -common_.t_.offset(),
                                -common_.t_.offset());
            }
            else if (!transparent_is_identity && st.comp_op())
            {
                composite(previous_buffer,
                          current_buffer,
                          *st.comp_op(),
                          st.get_opacity(),
                          -common_.t_.offset(),
                          -common_.t_.offset());
            }
            previous_buffer.painted(previous_buffer.painted() || current_buffer.painted());
            if (transparent_is_identity)
            {
                clear_spans(current_buffer, spans);
                current_buffer.painted(false);
            }
        }
        if (internal_buffers_.in_range() && &current_buffer == &internal_buffers_.top())
        {
            internal_buffers_.pop();
        }
    }
    if (st.direct_image_filters().size() > 0)
    {
        // apply any 'direct' image filters
        mapnik::filter::filter_visitor<buffer_type> visitor(previous_buffer, common_.scale_factor_);
        for (mapnik::filter::filter_type const& filter_tag : st.direct_image_filters())
        {
            util::apply_visitor(visitor, filter_tag);
        }
        mapnik::premultiply_alpha(previous_buffer);
        previous_buffer.painted(true);
    }
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: End processing style";
}

template<typename buffer_type>
struct agg_render_marker_visitor
{
    agg_render_marker_visitor(renderer_common& common,
                              buffer_type& current_buffer,
                              std::unique_ptr<rasterizer> const& ras_ptr,
                              gamma_method_enum& gamma_method,
                              double& gamma,
                              pixel_position const& pos,
                              agg::trans_affine const& tr,
                              double opacity,
                              composite_mode_e comp_op)
        : common_(common),
          current_buffer_(current_buffer),
          ras_ptr_(ras_ptr),
          gamma_method_(gamma_method),
          gamma_(gamma),
          pos_(pos),
          tr_(tr),
          opacity_(opacity),
          comp_op_(comp_op)
    {}

    void operator()(marker_null const&) const {}

    void operator()(marker_svg const& marker) const
    {
        using color_type = agg::rgba8;
        using order_type = agg::order_rgba;
        using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
        using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
        using renderer_base = agg::renderer_base<pixfmt_comp_type>;
        using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;

        ras_ptr_->reset();
        if (gamma_method_ != gamma_method_enum::GAMMA_POWER || gamma_ != 1.0)
        {
            ras_ptr_->gamma(agg::gamma_power());
            gamma_method_ = gamma_method_enum::GAMMA_POWER;
            gamma_ = 1.0;
        }
        agg::scanline_u8 sl;
        agg::rendering_buffer buf(current_buffer_.bytes(),
                                  current_buffer_.width(),
                                  current_buffer_.height(),
                                  current_buffer_.row_size());
        pixfmt_comp_type pixf(buf);
        pixf.comp_op(static_cast<agg::comp_op_e>(comp_op_));
        renderer_base renb(pixf);

        box2d<double> const& bbox = marker.get_data()->bounding_box();
        coord<double, 2> c = bbox.center();
        // center the svg marker on '0,0'
        agg::trans_affine mtx = agg::trans_affine_translation(-c.x, -c.y);
        // apply symbol transformation to get to map space
        mtx *= tr_;
        mtx *= agg::trans_affine_scaling(common_.scale_factor_);
        // render the marker at the center of the marker box
        mtx.translate(pos_.x, pos_.y);
        svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(marker.get_data()->source());
        svg_path_adapter svg_path(stl_storage);
        svg::renderer_agg<svg_path_adapter, svg_attribute_type, renderer_type, pixfmt_comp_type> svg_renderer(
          svg_path,
          marker.get_data()->svg_group());

        // https://github.com/mapnik/mapnik/issues/1316
        // https://github.com/mapnik/mapnik/issues/1866
        mtx.tx = std::floor(mtx.tx + .5);
        mtx.ty = std::floor(mtx.ty + .5);
        svg_renderer.render(*ras_ptr_, sl, renb, mtx, opacity_, bbox);
    }

    void operator()(marker_rgba8 const& marker) const
    {
        using color_type = agg::rgba8;
        using order_type = agg::order_rgba;
        using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
        using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
        using renderer_base = agg::renderer_base<pixfmt_comp_type>;

        ras_ptr_->reset();
        if (gamma_method_ != gamma_method_enum::GAMMA_POWER || gamma_ != 1.0)
        {
            ras_ptr_->gamma(agg::gamma_power());
            gamma_method_ = gamma_method_enum::GAMMA_POWER;
            gamma_ = 1.0;
        }
        agg::scanline_u8 sl;
        agg::rendering_buffer buf(current_buffer_.bytes(),
                                  current_buffer_.width(),
                                  current_buffer_.height(),
                                  current_buffer_.row_size());
        pixfmt_comp_type pixf(buf);
        pixf.comp_op(static_cast<agg::comp_op_e>(comp_op_));
        renderer_base renb(pixf);

        double width = marker.width();
        double height = marker.height();
        if (std::fabs(1.0 - common_.scale_factor_) < 0.001 && (std::fabs(1.0 - tr_.sx) < agg::affine_epsilon) &&
            (std::fabs(0.0 - tr_.shy) < agg::affine_epsilon) && (std::fabs(0.0 - tr_.shx) < agg::affine_epsilon) &&
            (std::fabs(1.0 - tr_.sy) < agg::affine_epsilon))
        {
            double cx = 0.5 * width;
            double cy = 0.5 * height;
            composite(current_buffer_,
                      marker.get_data(),
                      comp_op_,
                      opacity_,
                      std::floor(pos_.x - cx + .5),
                      std::floor(pos_.y - cy + .5));
        }
        else
        {
            double p[8];
            double x0 = pos_.x - 0.5 * width;
            double y0 = pos_.y - 0.5 * height;
            p[0] = x0;
            p[1] = y0;
            p[2] = x0 + width;
            p[3] = y0;
            p[4] = x0 + width;
            p[5] = y0 + height;
            p[6] = x0;
            p[7] = y0 + height;

            agg::trans_affine marker_tr;

            marker_tr *= agg::trans_affine_translation(-pos_.x, -pos_.y);
            marker_tr *= tr_;
            marker_tr *= agg::trans_affine_scaling(common_.scale_factor_);
            marker_tr *= agg::trans_affine_translation(pos_.x, pos_.y);

            marker_tr.transform(&p[0], &p[1]);
            marker_tr.transform(&p[2], &p[3]);
            marker_tr.transform(&p[4], &p[5]);
            marker_tr.transform(&p[6], &p[7]);

            ras_ptr_->move_to_d(p[0], p[1]);
            ras_ptr_->line_to_d(p[2], p[3]);
            ras_ptr_->line_to_d(p[4], p[5]);
            ras_ptr_->line_to_d(p[6], p[7]);

            agg::span_allocator<color_type> sa;
            agg::image_filter_bilinear filter_kernel;
            agg::image_filter_lut filter(filter_kernel, false);

            buffer_type const& src = marker.get_data();
            agg::rendering_buffer marker_buf((unsigned char*)src.bytes(), src.width(), src.height(), src.row_size());
            agg::pixfmt_rgba32_pre marker_pixf(marker_buf);
            using img_accessor_type = agg::image_accessor_clone<agg::pixfmt_rgba32_pre>;
            using interpolator_type = agg::span_interpolator_linear<agg::trans_affine>;
            using span_gen_type = agg::span_image_filter_rgba_2x2<img_accessor_type, interpolator_type>;
            using renderer_type =
              agg::renderer_scanline_aa_alpha<renderer_base, agg::span_allocator<agg::rgba8>, span_gen_type>;
            img_accessor_type ia(marker_pixf);
            agg::trans_affine final_tr(p, 0, 0, width, height);
            final_tr.tx = std::floor(final_tr.tx + .5);
            final_tr.ty = std::floor(final_tr.ty + .5);
            interpolator_type interpolator(final_tr);
            span_gen_type sg(ia, interpolator, filter);
            renderer_type rp(renb, sa, sg, unsigned(opacity_ * 255));
            agg::render_scanlines(*ras_ptr_, sl, rp);
        }
    }

  private:
    renderer_common& common_;
    buffer_type& current_buffer_;
    std::unique_ptr<rasterizer> const& ras_ptr_;
    gamma_method_enum& gamma_method_;
    double& gamma_;
    pixel_position const& pos_;
    agg::trans_affine const& tr_;
    double opacity_;
    composite_mode_e comp_op_;
};

template<typename T0, typename T1>
void agg_renderer<T0, T1>::render_marker(pixel_position const& pos,
                                         marker const& marker,
                                         agg::trans_affine const& tr,
                                         double opacity,
                                         composite_mode_e comp_op)
{
    agg_render_marker_visitor<buffer_type>
      visitor(common_, buffers_.top().get(), ras_ptr, gamma_method_, gamma_, pos, tr, opacity, comp_op);
    util::apply_visitor(visitor, marker);
}

template<typename T0, typename T1>
bool agg_renderer<T0, T1>::painted()
{
    return buffers_.top().get().painted();
}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::painted(bool painted)
{
    buffers_.top().get().painted(painted);
}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::debug_draw_box(box2d<double> const& box, double x, double y, double angle)
{
    buffer_type& current_buffer = buffers_.top().get();
    agg::rendering_buffer buf(current_buffer.bytes(),
                              current_buffer.width(),
                              current_buffer.height(),
                              current_buffer.row_size());
    debug_draw_box(buf, box, x, y, angle);
}

template<typename T0, typename T1>
template<typename R>
void agg_renderer<T0, T1>::debug_draw_box(R& buf, box2d<double> const& box, double x, double y, double angle)
{
    using pixfmt = agg::pixfmt_rgba32_pre;
    using renderer_base = agg::renderer_base<pixfmt>;
    using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;

    agg::scanline_p8 sl_line;
    pixfmt pixf(buf);
    renderer_base renb(pixf);
    renderer_type ren(renb);

    // compute transformation matrix
    agg::trans_affine tr = agg::trans_affine_rotation(angle).translate(x, y);
    // prepare path
    agg::path_storage pbox;
    pbox.start_new_path();
    pbox.move_to(box.minx(), box.miny());
    pbox.line_to(box.maxx(), box.miny());
    pbox.line_to(box.maxx(), box.maxy());
    pbox.line_to(box.minx(), box.maxy());
    pbox.line_to(box.minx(), box.miny());

    // prepare stroke with applied transformation
    using conv_transform = agg::conv_transform<agg::path_storage>;
    using conv_stroke = agg::conv_stroke<conv_transform>;
    conv_transform tbox(pbox, tr);
    conv_stroke sbox(tbox);
    sbox.generator().width(1.0 * common_.scale_factor_);

    // render the outline
    ras_ptr->reset();
    ras_ptr->add_path(sbox);
    ren.color(agg::rgba8_pre(0x33, 0x33, 0xff, 0xcc)); // blue is fine
    agg::render_scanlines(*ras_ptr, sl_line, ren);
}

template<typename T0, typename T1>
void agg_renderer<T0, T1>::draw_geo_extent(box2d<double> const& extent, mapnik::color const& color)
{
    box2d<double> box = common_.t_.forward(extent);
    double x0 = box.minx();
    double x1 = box.maxx();
    double y0 = box.miny();
    double y1 = box.maxy();
    unsigned rgba = color.rgba();
    for (double x = x0; x < x1; x++)
    {
        mapnik::set_pixel(buffers_.top().get(), x, y0, rgba);
        mapnik::set_pixel(buffers_.top().get(), x, y1, rgba);
    }
    for (double y = y0; y < y1; y++)
    {
        mapnik::set_pixel(buffers_.top().get(), x0, y, rgba);
        mapnik::set_pixel(buffers_.top().get(), x1, y, rgba);
    }
}

template class agg_renderer<image_rgba8>;
template void agg_renderer<image_rgba8>::debug_draw_box<agg::rendering_buffer>(agg::rendering_buffer& buf,
                                                                               box2d<double> const& box,
                                                                               double x,
                                                                               double y,
                                                                               double angle);
} // namespace mapnik
