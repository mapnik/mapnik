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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/cairo_render_vector.hpp>
#include <mapnik/map.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/request.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/feature_type_style.hpp>

// agg
#include "agg/include/agg_trans_affine.h"  // for trans_affine, etc

// stl
#include <cmath>

namespace mapnik
{

class feature_type_style;

template <typename T>
cairo_renderer<T>::cairo_renderer(Map const& m,
                                  T const& cairo,
                                  double scale_factor,
                                  unsigned offset_x,
                                  unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m, scale_factor),
      m_(m),
      context_(cairo),
      common_(m, attributes(), offset_x, offset_y, m.width(), m.height(), scale_factor),
      face_manager_(common_.shared_font_library_),
      style_level_compositing_(false)
{
    setup(m);
}

template <typename T>
cairo_renderer<T>::cairo_renderer(Map const& m,
                                  request const& req,
                                  attributes const& vars,
                                  T const& cairo,
                                  double scale_factor,
                                  unsigned offset_x,
                                  unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m, scale_factor),
      m_(m),
      context_(cairo),
      common_(m, req, vars, offset_x, offset_y, req.width(), req.height(), scale_factor),
      face_manager_(common_.shared_font_library_),
      style_level_compositing_(false)

{
    setup(m);
}

template <typename T>
cairo_renderer<T>::cairo_renderer(Map const& m,
                                 T const& cairo,
                                 std::shared_ptr<label_collision_detector4> detector,
                                 double scale_factor,
                                 unsigned offset_x,
                                 unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m, scale_factor),
      m_(m),
      context_(cairo),
      common_(m, attributes(), offset_x, offset_y, m.width(), m.height(), scale_factor, detector),
      face_manager_(common_.shared_font_library_),
      style_level_compositing_(false)

{
    setup(m);
}

template <typename T>
cairo_renderer<T>::~cairo_renderer() {}

struct setup_marker_visitor
{
    setup_marker_visitor(cairo_context & context, renderer_common const& common)
        : context_(context), common_(common) {}

    void operator() (marker_null const &) const{}
    void operator() (marker_svg const &) const {}

    void operator() (marker_rgba8 const& marker) const
    {
        mapnik::image_rgba8 const& bg_image = marker.get_data();
        std::size_t w = bg_image.width();
        std::size_t h = bg_image.height();
        if ( w > 0 && h > 0)
        {
            // repeat background-image both vertically and horizontally
            std::size_t x_steps = std::size_t(std::ceil(common_.width_/double(w)));
            std::size_t y_steps = std::size_t(std::ceil(common_.height_/double(h)));
            for (std::size_t x=0;x<x_steps;++x)
            {
                for (std::size_t y=0;y<y_steps;++y)
                {
                    agg::trans_affine matrix = agg::trans_affine_translation(
                                                   x*w,
                                                   y*h);
                    context_.add_image(matrix, bg_image, 1.0f);
                }
            }
        }
    }

  private:
    cairo_context & context_;
    renderer_common const& common_;
};

template <typename T>
void cairo_renderer<T>::setup(Map const& map)
{
    boost::optional<color> bg = m_.background();
    if (bg)
    {
        cairo_save_restore guard(context_);
        context_.set_color(*bg);
        context_.set_operator(composite_mode_e::src);
        context_.paint();
    }
    boost::optional<std::string> const& image_filename = map.background_image();
    if (image_filename)
    {
        // NOTE: marker_cache returns premultiplied image, if needed
        std::shared_ptr<mapnik::marker const> bg_marker = mapnik::marker_cache::instance().find(*image_filename,true);
        util::apply_visitor(setup_marker_visitor(context_, common_), *bg_marker);
    }
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer: Scale=" << map.scale();
}

template <typename T>
void cairo_renderer<T>::start_map_processing(Map const& map)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer: Start map processing bbox=" << map.get_current_extent();
    box2d<double> bounds = common_.t_.forward(common_.t_.extent());
    context_.rectangle(bounds.minx(), bounds.miny(), bounds.maxx(), bounds.maxy());
    context_.clip();
}

template <typename T>
void cairo_renderer<T>::end_map_processing(Map const&)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer: End map processing";
}

template <typename T>
void cairo_renderer<T>::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer: Start processing layer=" << lay.name() ;
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer: -- datasource=" << lay.datasource().get();
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer: -- query_extent=" << query_extent;

    if (lay.clear_label_cache())
    {
        common_.detector_->clear();
    }
    common_.query_extent_ = query_extent;

    if (lay.comp_op() || lay.get_opacity() < 1.0)
    {
        context_.push_group();
    }
}

template <typename T>
void cairo_renderer<T>::end_layer_processing(layer const& lay)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer: End layer processing";

    if (lay.comp_op() || lay.get_opacity() < 1.0)
    {
        context_.pop_group();
        composite_mode_e comp_op = lay.comp_op() ? *lay.comp_op() : src_over;
        context_.set_operator(comp_op);
        context_.paint(lay.get_opacity());
    }
}

template <typename T>
void cairo_renderer<T>::start_style_processing(feature_type_style const & st)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer:start style processing";

    style_level_compositing_ = st.comp_op() || st.get_opacity() < 1;

    if (style_level_compositing_)
    {
        context_.push_group();
    }
}

template <typename T>
void cairo_renderer<T>::end_style_processing(feature_type_style const & st)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer:end style processing";

    if (style_level_compositing_)
    {
        context_.pop_group();
        composite_mode_e comp_op = st.comp_op() ? *st.comp_op() : src_over;
        context_.set_operator(comp_op);
        context_.paint(st.get_opacity());
    }
}

struct cairo_render_marker_visitor
{
    cairo_render_marker_visitor(cairo_context & context,
                                renderer_common const& common,
                                pixel_position const& pos,
                                agg::trans_affine const& tr,
                                double opacity,
                                bool recenter)
        : context_(context),
          common_(common),
          pos_(pos),
          tr_(tr),
          opacity_(opacity),
          recenter_(recenter) {}

    void operator() (marker_null const&) {}

    void operator() (marker_svg const& marker)
    {
        mapnik::svg_path_ptr vmarker = marker.get_data();
        if (vmarker)
        {
            box2d<double> bbox = vmarker->bounding_box();
            agg::trans_affine marker_tr = tr_;
            if (recenter_)
            {
                coord<double,2> c = bbox.center();
                marker_tr = agg::trans_affine_translation(-c.x,-c.y);
                marker_tr *= tr_;
            }
            marker_tr *= agg::trans_affine_scaling(common_.scale_factor_);
            agg::pod_bvector<svg::path_attributes> const & attrs = vmarker->attributes();
            svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(vmarker->source());
            svg::svg_path_adapter svg_path(stl_storage);
            marker_tr.translate(pos_.x, pos_.y);
            render_vector_marker(context_, svg_path, attrs, bbox, marker_tr, opacity_);
        }
    }

    void operator() (marker_rgba8 const& marker)
    {
        double width = marker.get_data().width();
        double height = marker.get_data().height();
        double cx = 0.5 * width;
        double cy = 0.5 * height;
        agg::trans_affine marker_tr;
        marker_tr *= agg::trans_affine_translation(-cx,-cy);
        marker_tr *= tr_;
        marker_tr *= agg::trans_affine_scaling(common_.scale_factor_);
        marker_tr *= agg::trans_affine_translation(pos_.x,pos_.y);
        context_.add_image(marker_tr, marker.get_data(), opacity_);
    }

  private:
    cairo_context & context_;
    renderer_common const& common_;
    pixel_position const& pos_;
    agg::trans_affine const& tr_;
    double opacity_;
    bool recenter_;
};

template <typename T>
void cairo_renderer<T>::render_marker(pixel_position const& pos,
                                        marker const& marker,
                                        agg::trans_affine const& tr,
                                        double opacity,
                                        bool recenter)

{
    cairo_save_restore guard(context_);
    cairo_render_marker_visitor visitor(context_,
                                        common_,
                                        pos,
                                        tr,
                                        opacity,
                                        recenter);
    util::apply_visitor(visitor, marker);
}

template class cairo_renderer<cairo_ptr>;

}

#endif // HAVE_CAIRO
