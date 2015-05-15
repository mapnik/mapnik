/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/image_util.hpp>
#include <mapnik/image_any.hpp>
// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_scanline_u.h"
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"

// boost
#include <boost/optional.hpp>

// stl
#include <cmath>

namespace mapnik
{

template <typename T0, typename T1>
agg_renderer<T0,T1>::agg_renderer(Map const& m, T0 & pixmap, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<agg_renderer>(m, scale_factor),
      pixmap_(pixmap),
      internal_buffer_(),
      current_buffer_(&pixmap),
      style_level_compositing_(false),
      ras_ptr(new rasterizer),
      gamma_method_(GAMMA_POWER),
      gamma_(1.0),
      common_(m, attributes(), offset_x, offset_y, m.width(), m.height(), scale_factor)
{
    setup(m);
}

template <typename T0, typename T1>
agg_renderer<T0,T1>::agg_renderer(Map const& m, request const& req, attributes const& vars, T0 & pixmap, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<agg_renderer>(m, scale_factor),
      pixmap_(pixmap),
      internal_buffer_(),
      current_buffer_(&pixmap),
      style_level_compositing_(false),
      ras_ptr(new rasterizer),
      gamma_method_(GAMMA_POWER),
      gamma_(1.0),
      common_(m, req, vars, offset_x, offset_y, req.width(), req.height(), scale_factor)
{
    setup(m);
}

template <typename T0, typename T1>
agg_renderer<T0,T1>::agg_renderer(Map const& m, T0 & pixmap, std::shared_ptr<T1> detector,
                              double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<agg_renderer>(m, scale_factor),
      pixmap_(pixmap),
      internal_buffer_(),
      current_buffer_(&pixmap),
      style_level_compositing_(false),
      ras_ptr(new rasterizer),
      gamma_method_(GAMMA_POWER),
      gamma_(1.0),
      common_(m, attributes(), offset_x, offset_y, m.width(), m.height(), scale_factor, detector)
{
    setup(m);
}

template <typename buffer_type>
struct setup_agg_bg_visitor
{
    setup_agg_bg_visitor(buffer_type & pixmap,
                         renderer_common const& common,
                         composite_mode_e mode,
                         double opacity)
        :  pixmap_(pixmap),
           common_(common),
           mode_(mode),
           opacity_(opacity) {}

    void operator() (marker_null const&) {}

    void operator() (marker_svg const&) {}

    void operator() (marker_rgba8 const& marker)
    {
        mapnik::image_rgba8 const& bg_image = marker.get_data();
        int w = bg_image.width();
        int h = bg_image.height();
        if ( w > 0 && h > 0)
        {
            // repeat background-image both vertically and horizontally
            unsigned x_steps = static_cast<unsigned>(std::ceil(common_.width_/double(w)));
            unsigned y_steps = static_cast<unsigned>(std::ceil(common_.height_/double(h)));
            for (unsigned x=0;x<x_steps;++x)
            {
                for (unsigned y=0;y<y_steps;++y)
                {
                    composite(pixmap_, bg_image, mode_, opacity_, x*w, y*h);
                }
            }
        }
    }
  private:
    buffer_type & pixmap_;
    renderer_common const& common_;
    composite_mode_e mode_;
    double opacity_;
};

template <typename T0, typename T1>
void agg_renderer<T0,T1>::setup(Map const &m)
{
    mapnik::set_premultiplied_alpha(pixmap_, true);
    boost::optional<color> const& bg = m.background();
    if (bg)
    {
        if (bg->alpha() < 255)
        {
            mapnik::color bg_color = *bg;
            bg_color.premultiply();
            mapnik::fill(pixmap_, bg_color);
        }
        else
        {
            mapnik::color bg_color = *bg;
            bg_color.set_premultiplied(true);
            mapnik::fill(pixmap_,bg_color);
        }
    }

    boost::optional<std::string> const& image_filename = m.background_image();
    if (image_filename)
    {
        // NOTE: marker_cache returns premultiplied image, if needed
        std::shared_ptr<mapnik::marker const> bg_marker = mapnik::marker_cache::instance().find(*image_filename,true);
        setup_agg_bg_visitor<buffer_type> visitor(pixmap_,
                                     common_,
                                     m.background_image_comp_op(),
                                     m.background_image_opacity());
        util::apply_visitor(visitor, *bg_marker);
    }
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Scale=" << m.scale();
}

template <typename T0, typename T1>
agg_renderer<T0,T1>::~agg_renderer() {}

template <typename T0, typename T1>
void agg_renderer<T0,T1>::start_map_processing(Map const& map)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Start map processing bbox=" << map.get_current_extent();
    ras_ptr->clip_box(0,0,common_.width_,common_.height_);
}

template <typename T0, typename T1>
void agg_renderer<T0,T1>::end_map_processing(Map const& )
{
    mapnik::demultiply_alpha(pixmap_);
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: End map processing";
}

template <typename T0, typename T1>
void agg_renderer<T0,T1>::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Start processing layer=" << lay.name();
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: -- datasource=" << lay.datasource().get();
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: -- query_extent=" << query_extent;

    if (lay.clear_label_cache())
    {
        common_.detector_->clear();
    }

    common_.query_extent_ = query_extent;
    boost::optional<box2d<double> > const& maximum_extent = lay.maximum_extent();
    if (maximum_extent)
    {
        common_.query_extent_.clip(*maximum_extent);
    }
}

template <typename T0, typename T1>
void agg_renderer<T0,T1>::end_layer_processing(layer const&)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: End layer processing";
}

template <typename T0, typename T1>
void agg_renderer<T0,T1>::start_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Start processing style";
    if (st.comp_op() || st.image_filters().size() > 0 || st.get_opacity() < 1)
    {
        style_level_compositing_ = true;
    }
    else
    {
        style_level_compositing_ = false;
    }

    if (style_level_compositing_)
    {
        if (st.image_filters_inflate())
        {
            int radius = 0;
            mapnik::filter::filter_radius_visitor visitor(radius);
            for (mapnik::filter::filter_type const& filter_tag : st.image_filters())
            {
                util::apply_visitor(visitor, filter_tag);
            }
            if (radius > common_.t_.offset())
            {
                common_.t_.set_offset(radius);
            }
            int offset = common_.t_.offset();
            unsigned target_width = common_.width_ + (offset * 2);
            unsigned target_height = common_.height_ + (offset * 2);
            ras_ptr->clip_box(-int(offset*2),-int(offset*2),target_width,target_height);
            if (!internal_buffer_ ||
               (internal_buffer_->width() < target_width ||
                internal_buffer_->height() < target_height))
            {
                internal_buffer_ = std::make_shared<buffer_type>(target_width,target_height);
            }
            else
            {
                mapnik::fill(*internal_buffer_, 0); // fill with transparent colour
            }
        }
        else
        {
            if (!internal_buffer_)
            {
                internal_buffer_ = std::make_shared<buffer_type>(common_.width_,common_.height_);
            }
            else
            {
                mapnik::fill(*internal_buffer_, 0); // fill with transparent colour
            }
            common_.t_.set_offset(0);
            ras_ptr->clip_box(0,0,common_.width_,common_.height_);
        }
        current_buffer_ = internal_buffer_.get();
        set_premultiplied_alpha(*current_buffer_,true);
    }
    else
    {
        common_.t_.set_offset(0);
        ras_ptr->clip_box(0,0,common_.width_,common_.height_);
        current_buffer_ = &pixmap_;
    }
}

template <typename T0, typename T1>
void agg_renderer<T0,T1>::end_style_processing(feature_type_style const& st)
{
    if (style_level_compositing_)
    {
        bool blend_from = false;
        if (st.image_filters().size() > 0)
        {
            blend_from = true;
            mapnik::filter::filter_visitor<buffer_type> visitor(*current_buffer_);
            for (mapnik::filter::filter_type const& filter_tag : st.image_filters())
            {
                util::apply_visitor(visitor, filter_tag);
            }
        }
        if (st.comp_op())
        {
            composite(pixmap_, *current_buffer_,
                      *st.comp_op(), st.get_opacity(),
                      -common_.t_.offset(),
                      -common_.t_.offset());
        }
        else if (blend_from || st.get_opacity() < 1.0)
        {
            composite(pixmap_, *current_buffer_,
                      src_over, st.get_opacity(),
                      -common_.t_.offset(),
                      -common_.t_.offset());
        }
    }
    // apply any 'direct' image filters
    mapnik::filter::filter_visitor<buffer_type> visitor(pixmap_);
    for (mapnik::filter::filter_type const& filter_tag : st.direct_image_filters())
    {
        util::apply_visitor(visitor, filter_tag);
    }
    MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: End processing style";
}

template <typename buffer_type>
struct agg_render_marker_visitor
{
    agg_render_marker_visitor(renderer_common & common,
                              buffer_type * current_buffer,
                              std::unique_ptr<rasterizer> const& ras_ptr,
                              gamma_method_enum & gamma_method,
                              double & gamma,
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
          comp_op_(comp_op) {}

    void operator() (marker_null const&) {}

    void operator() (marker_svg const& marker)
    {
        using color_type = agg::rgba8;
        using order_type = agg::order_rgba;
        using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
        using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
        using renderer_base = agg::renderer_base<pixfmt_comp_type>;
        using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;
        using svg_attribute_type = agg::pod_bvector<mapnik::svg::path_attributes>;

        ras_ptr_->reset();
        if (gamma_method_ != GAMMA_POWER || gamma_ != 1.0)
        {
            ras_ptr_->gamma(agg::gamma_power());
            gamma_method_ = GAMMA_POWER;
            gamma_ = 1.0;
        }
        agg::scanline_u8 sl;
        agg::rendering_buffer buf(current_buffer_->bytes(),
                                  current_buffer_->width(),
                                  current_buffer_->height(),
                                  current_buffer_->row_size());
        pixfmt_comp_type pixf(buf);
        pixf.comp_op(static_cast<agg::comp_op_e>(comp_op_));
        renderer_base renb(pixf);

        box2d<double> const& bbox = marker.get_data()->bounding_box();
        coord<double,2> c = bbox.center();
        // center the svg marker on '0,0'
        agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
        // apply symbol transformation to get to map space
        mtx *= tr_;
        mtx *= agg::trans_affine_scaling(common_.scale_factor_);
        // render the marker at the center of the marker box
        mtx.translate(pos_.x, pos_.y);
        using namespace mapnik::svg;
        vertex_stl_adapter<svg_path_storage> stl_storage(marker.get_data()->source());
        svg_path_adapter svg_path(stl_storage);
        svg_renderer_agg<svg_path_adapter,
                         svg_attribute_type,
                         renderer_type,
                         pixfmt_comp_type> svg_renderer(svg_path,
                                                        marker.get_data()->attributes());

        // https://github.com/mapnik/mapnik/issues/1316
        // https://github.com/mapnik/mapnik/issues/1866
        mtx.tx = std::floor(mtx.tx+.5);
        mtx.ty = std::floor(mtx.ty+.5);
        svg_renderer.render(*ras_ptr_, sl, renb, mtx, opacity_, bbox);
    }

    void operator() (marker_rgba8 const& marker)
    {
        using color_type = agg::rgba8;
        using order_type = agg::order_rgba;
        using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
        using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
        using renderer_base = agg::renderer_base<pixfmt_comp_type>;

        ras_ptr_->reset();
        if (gamma_method_ != GAMMA_POWER || gamma_ != 1.0)
        {
            ras_ptr_->gamma(agg::gamma_power());
            gamma_method_ = GAMMA_POWER;
            gamma_ = 1.0;
        }
        agg::scanline_u8 sl;
        agg::rendering_buffer buf(current_buffer_->bytes(),
                                  current_buffer_->width(),
                                  current_buffer_->height(),
                                  current_buffer_->row_size());
        pixfmt_comp_type pixf(buf);
        pixf.comp_op(static_cast<agg::comp_op_e>(comp_op_));
        renderer_base renb(pixf);

        double width = marker.width();
        double height = marker.height();
        if (std::fabs(1.0 - common_.scale_factor_) < 0.001
            && (std::fabs(1.0 - tr_.sx) < agg::affine_epsilon)
            && (std::fabs(0.0 - tr_.shy) < agg::affine_epsilon)
            && (std::fabs(0.0 - tr_.shx) < agg::affine_epsilon)
            && (std::fabs(1.0 - tr_.sy) < agg::affine_epsilon))
        {
            double cx = 0.5 * width;
            double cy = 0.5 * height;
            composite(*current_buffer_, marker.get_data(),
                      comp_op_, opacity_,
                      std::floor(pos_.x - cx + .5),
                      std::floor(pos_.y - cy + .5));
        }
        else
        {

            double p[8];
            double x0 = pos_.x - 0.5 * width;
            double y0 = pos_.y - 0.5 * height;
            p[0] = x0;         p[1] = y0;
            p[2] = x0 + width; p[3] = y0;
            p[4] = x0 + width; p[5] = y0 + height;
            p[6] = x0;         p[7] = y0 + height;

            agg::trans_affine marker_tr;

            marker_tr *= agg::trans_affine_translation(-pos_.x,-pos_.y);
            marker_tr *= tr_;
            marker_tr *= agg::trans_affine_scaling(common_.scale_factor_);
            marker_tr *= agg::trans_affine_translation(pos_.x,pos_.y);

            marker_tr.transform(&p[0], &p[1]);
            marker_tr.transform(&p[2], &p[3]);
            marker_tr.transform(&p[4], &p[5]);
            marker_tr.transform(&p[6], &p[7]);

            ras_ptr_->move_to_d(p[0],p[1]);
            ras_ptr_->line_to_d(p[2],p[3]);
            ras_ptr_->line_to_d(p[4],p[5]);
            ras_ptr_->line_to_d(p[6],p[7]);


            agg::span_allocator<color_type> sa;
            agg::image_filter_bilinear filter_kernel;
            agg::image_filter_lut filter(filter_kernel, false);

            buffer_type const& src = marker.get_data();
            agg::rendering_buffer marker_buf((unsigned char *)src.bytes(),
                                             src.width(),
                                             src.height(),
                                             src.row_size());
            agg::pixfmt_rgba32_pre marker_pixf(marker_buf);
            using img_accessor_type = agg::image_accessor_clone<agg::pixfmt_rgba32_pre>;
            using interpolator_type = agg::span_interpolator_linear<agg::trans_affine>;
            using span_gen_type = agg::span_image_filter_rgba_2x2<img_accessor_type,
                                                                  interpolator_type>;
            using renderer_type = agg::renderer_scanline_aa_alpha<renderer_base,
                                                                  agg::span_allocator<agg::rgba8>,
                                                                  span_gen_type>;
            img_accessor_type ia(marker_pixf);
            agg::trans_affine final_tr(p, 0, 0, width, height);
            final_tr.tx = std::floor(final_tr.tx+.5);
            final_tr.ty = std::floor(final_tr.ty+.5);
            interpolator_type interpolator(final_tr);
            span_gen_type sg(ia, interpolator, filter);
            renderer_type rp(renb,sa, sg, unsigned(opacity_*255));
            agg::render_scanlines(*ras_ptr_, sl, rp);
        }
    }

  private:
    renderer_common & common_;
    buffer_type * current_buffer_;
    std::unique_ptr<rasterizer> const& ras_ptr_;
    gamma_method_enum & gamma_method_;
    double & gamma_;
    pixel_position const& pos_;
    agg::trans_affine const& tr_;
    double opacity_;
    composite_mode_e comp_op_;

};

template <typename T0, typename T1>
void agg_renderer<T0,T1>::render_marker(pixel_position const& pos,
                                    marker const& marker,
                                    agg::trans_affine const& tr,
                                    double opacity,
                                    composite_mode_e comp_op)
{
    agg_render_marker_visitor<buffer_type> visitor(common_,
                                                   current_buffer_,
                                                   ras_ptr,
                                                   gamma_method_,
                                                   gamma_,
                                                   pos,
                                                   tr,
                                                   opacity,
                                                   comp_op);
    util::apply_visitor(visitor, marker);
}

template <typename T0, typename T1>
bool agg_renderer<T0,T1>::painted()
{
    return pixmap_.painted();
}

template <typename T0, typename T1>
void agg_renderer<T0,T1>::painted(bool painted)
{
    pixmap_.painted(painted);
}

template <typename T0, typename T1>
void agg_renderer<T0,T1>::debug_draw_box(box2d<double> const& box,
                                     double x, double y, double angle)
{
    agg::rendering_buffer buf(current_buffer_->bytes(),
                              current_buffer_->width(),
                              current_buffer_->height(),
                              current_buffer_->row_size());
    debug_draw_box(buf, box, x, y, angle);
}

template <typename T0, typename T1> template <typename R>
void agg_renderer<T0,T1>::debug_draw_box(R& buf, box2d<double> const& box,
                                     double x, double y, double angle)
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

template <typename T0, typename T1>
void agg_renderer<T0,T1>::draw_geo_extent(box2d<double> const& extent, mapnik::color const& color)
{
    box2d<double> box = common_.t_.forward(extent);
    double x0 = box.minx();
    double x1 = box.maxx();
    double y0 = box.miny();
    double y1 = box.maxy();
    unsigned rgba = color.rgba();
    for (double x=x0; x<x1; x++)
    {
        mapnik::set_pixel(pixmap_, x, y0, rgba);
        mapnik::set_pixel(pixmap_, x, y1, rgba);
    }
    for (double y=y0; y<y1; y++)
    {
        mapnik::set_pixel(pixmap_, x0, y, rgba);
        mapnik::set_pixel(pixmap_, x1, y, rgba);
    }
}

template class agg_renderer<image_rgba8>;
template void agg_renderer<image_rgba8>::debug_draw_box<agg::rendering_buffer>(
                agg::rendering_buffer& buf,
                box2d<double> const& box,
                double x, double y, double angle);
} // end ns
