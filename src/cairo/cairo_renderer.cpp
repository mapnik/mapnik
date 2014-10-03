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
      face_manager_(common_.shared_font_library_)
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
      face_manager_(common_.shared_font_library_)
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
      face_manager_(common_.shared_font_library_)
{
    setup(m);
}

template <typename T>
cairo_renderer<T>::~cairo_renderer() {}

template <typename T>
void cairo_renderer<T>::setup(Map const& map)
{
    boost::optional<color> bg = m_.background();
    if (bg)
    {
        cairo_save_restore guard(context_);
        context_.set_color(*bg);
        context_.paint();
    }
    boost::optional<std::string> const& image_filename = map.background_image();
    if (image_filename)
    {
        // NOTE: marker_cache returns premultiplied image, if needed
        boost::optional<mapnik::marker_ptr> bg_marker = mapnik::marker_cache::instance().find(*image_filename,true);
        if (bg_marker && (*bg_marker)->is_bitmap())
        {
            mapnik::image_ptr bg_image = *(*bg_marker)->get_bitmap_data();
            int w = bg_image->width();
            int h = bg_image->height();
            if ( w > 0 && h > 0)
            {
                // repeat background-image both vertically and horizontally
                unsigned x_steps = unsigned(std::ceil(common_.width_/double(w)));
                unsigned y_steps = unsigned(std::ceil(common_.height_/double(h)));
                for (unsigned x=0;x<x_steps;++x)
                {
                    for (unsigned y=0;y<y_steps;++y)
                    {
                        agg::trans_affine matrix = agg::trans_affine_translation(
                                                       x*w,
                                                       y*h);
                        context_.add_image(matrix, *bg_image, 1.0f);
                    }
                }
            }
        }
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
}

template <typename T>
void cairo_renderer<T>::end_layer_processing(layer const&)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer: End layer processing";
}

template <typename T>
void cairo_renderer<T>::start_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer:start style processing";
}

template <typename T>
void cairo_renderer<T>::end_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer:end style processing";
}

template <typename T>
void cairo_renderer<T>::render_marker(pixel_position const& pos,
                                        marker const& marker,
                                        agg::trans_affine const& tr,
                                        double opacity,
                                        bool recenter)

{
    cairo_save_restore guard(context_);
    if (marker.is_vector())
    {
        mapnik::svg_path_ptr vmarker = *marker.get_vector_data();
        if (vmarker)
        {
            agg::trans_affine marker_tr = tr;
            marker_tr *=agg::trans_affine_scaling(common_.scale_factor_);
            box2d<double> bbox = vmarker->bounding_box();
            agg::pod_bvector<svg::path_attributes> const & attributes = vmarker->attributes();
            svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(vmarker->source());
            svg::svg_path_adapter svg_path(stl_storage);
            render_vector_marker(context_, pos, svg_path, bbox, attributes, marker_tr, opacity, recenter);
        }
    }
    else if (marker.is_bitmap())
    {
        double width = (*marker.get_bitmap_data())->width();
        double height = (*marker.get_bitmap_data())->height();
        double cx = 0.5 * width;
        double cy = 0.5 * height;
        agg::trans_affine marker_tr;
        marker_tr *= agg::trans_affine_translation(-cx,-cy);
        marker_tr *= tr;
        marker_tr *= agg::trans_affine_scaling(common_.scale_factor_);
        marker_tr *= agg::trans_affine_translation(pos.x,pos.y);
        context_.add_image(marker_tr, **marker.get_bitmap_data(), opacity);
    }
}

template class cairo_renderer<cairo_ptr>;

}

#endif // HAVE_CAIRO
