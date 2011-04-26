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
//$Id$

// mapnik
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_pixfmt.hpp>
#include <mapnik/grid/grid_pixel.hpp>
#include <mapnik/grid/grid.hpp>

#include <mapnik/marker_cache.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>

// boost
#include <boost/utility.hpp>

// stl
#ifdef MAPNIK_DEBUG
#include <iostream>
#endif

//#include <cmath>

namespace mapnik
{

template <typename T>
grid_renderer<T>::grid_renderer(Map const& m, T & pixmap, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<grid_renderer>(m, scale_factor),
      pixmap_(pixmap),
      width_(pixmap_.width()),
      height_(pixmap_.height()),
      scale_factor_(scale_factor),
      t_(pixmap_.width(),pixmap_.width(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(),
      font_manager_(font_engine_),
      detector_(box2d<double>(-m.buffer_size(), -m.buffer_size(), pixmap_.width() + m.buffer_size(), pixmap_.height() + m.buffer_size())),
      ras_ptr(new grid_rasterizer)
{
#ifdef MAPNIK_DEBUG
    std::clog << "scale=" << m.scale() << "\n";
#endif
}

template <typename T>
grid_renderer<T>::~grid_renderer() {}

template <typename T>
void grid_renderer<T>::start_map_processing(Map const& map)
{
#ifdef MAPNIK_DEBUG
    std::clog << "start map processing bbox="
              << map.get_current_extent() << "\n";
#endif
    ras_ptr->clip_box(0,0,width_,height_);
}

template <typename T>
void grid_renderer<T>::end_map_processing(Map const& )
{
#ifdef MAPNIK_DEBUG
    std::clog << "end map processing\n";
#endif
}

template <typename T>
void grid_renderer<T>::start_layer_processing(layer const& lay)
{
#ifdef MAPNIK_DEBUG
    std::clog << "start layer processing : " << lay.name()  << "\n";
    std::clog << "datasource = " << lay.datasource().get() << "\n";
#endif
    if (lay.clear_label_cache())
    {
        detector_.clear();
    }
}

template <typename T>
void grid_renderer<T>::end_layer_processing(layer const&)
{
#ifdef MAPNIK_DEBUG
    std::clog << "end layer processing\n";
#endif
}

template <typename T>
void grid_renderer<T>::render_marker(Feature const& feature, unsigned int step, const int x, const int y, marker &marker, const agg::trans_affine & tr, double opacity)
{
    if (marker.is_vector())
    {
        typedef coord_transform2<CoordTransform,geometry_type> path_type;
        typedef agg::renderer_base<mapnik::pixfmt_gray16> ren_base;
        typedef agg::renderer_scanline_aa_solid<ren_base> renderer;
        agg::scanline_u8 sl;
    
        grid_rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_);
        mapnik::pixfmt_gray16 pixf(buf);
    
        ren_base renb(pixf);
        renderer ren(renb);
    
        ras_ptr->reset();
        ras_ptr->gamma(agg::gamma_linear(0.0, 0.0));


        box2d<double> const& bbox = (*marker.get_vector_data())->bounding_box();
        coord<double,2> c = bbox.center();
        // center the svg marker on '0,0'
        agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
        // apply symbol transformation to get to map space
        mtx *= tr;
        mtx *= agg::trans_affine_scaling(scale_factor_*(1.0/step));
        // render the marker at the center of the marker box
        mtx.translate(x+0.5 * marker.width(), y+0.5 * marker.height());

        vertex_stl_adapter<svg_path_storage> stl_storage((*marker.get_vector_data())->source());
        svg_path_adapter svg_path(stl_storage);
        svg_renderer<svg_path_adapter,
                     agg::pod_bvector<path_attributes>, mapnik::pixfmt_gray16 > svg_renderer(svg_path,
                             (*marker.get_vector_data())->attributes());

        svg_renderer.render_id(*ras_ptr, sl, renb, feature.id(), mtx, opacity, bbox);
        
    }
    else
    {
        pixmap_.set_rectangle(feature.id(), **marker.get_bitmap_data(), x, y);
    }
    pixmap_.add_feature(feature);
}

template class grid_renderer<grid>;
}
