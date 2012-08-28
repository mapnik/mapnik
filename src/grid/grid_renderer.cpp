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
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_pixfmt.hpp>
#include <mapnik/grid/grid_pixel.hpp>
#include <mapnik/grid/grid.hpp>

#include <mapnik/debug.hpp>
#include <mapnik/layer.hpp>
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

// boost
#include <boost/utility.hpp>
#include <boost/math/special_functions/round.hpp>

// agg
#include "agg_trans_affine.h"

namespace mapnik
{

template <typename T>
grid_renderer<T>::grid_renderer(Map const& m, T & pixmap, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<grid_renderer>(m, scale_factor),
      pixmap_(pixmap),
      width_(pixmap_.width()),
      height_(pixmap_.height()),
      scale_factor_(scale_factor),
      // NOTE: can change this to m dims instead of pixmap_ if render-time
      // resolution support is dropped from grid_renderer python interface
      t_(pixmap_.width(),pixmap_.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(),
      font_manager_(font_engine_),
      detector_(boost::make_shared<label_collision_detector4>(box2d<double>(-m.buffer_size(), -m.buffer_size(), m.width() + m.buffer_size() ,m.height() + m.buffer_size()))),
      ras_ptr(new grid_rasterizer)
{
    setup(m);
}

template <typename T>
void grid_renderer<T>::setup(Map const& m)
{
    MAPNIK_LOG_DEBUG(grid_renderer) << "grid_renderer: Scale=" << m.scale();
    // nothing to do for grids yet on setup
}

template <typename T>
grid_renderer<T>::~grid_renderer() {}

template <typename T>
void grid_renderer<T>::start_map_processing(Map const& m)
{
    MAPNIK_LOG_DEBUG(grid_renderer) << "grid_renderer: Start map processing bbox=" << m.get_current_extent();

    ras_ptr->clip_box(0,0,width_,height_);
}

template <typename T>
void grid_renderer<T>::end_map_processing(Map const& m)
{
    MAPNIK_LOG_DEBUG(grid_renderer) << "grid_renderer: End map processing";
}

template <typename T>
void grid_renderer<T>::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(grid_renderer) << "grid_renderer: Start processing layer=" << lay.name();
    MAPNIK_LOG_DEBUG(grid_renderer) << "grid_renderer: datasource=" << lay.datasource().get();
    MAPNIK_LOG_DEBUG(grid_renderer) << "grid_renderer: query_extent = " << query_extent;

    if (lay.clear_label_cache())
    {
        detector_->clear();
    }
    query_extent_ = query_extent;
    int buffer_size = lay.buffer_size();
    if (buffer_size != 0 )
    {
        double padding = buffer_size * (double)(query_extent.width()/pixmap_.width());
        double x0 = query_extent_.minx();
        double y0 = query_extent_.miny();
        double x1 = query_extent_.maxx();
        double y1 = query_extent_.maxy();
        query_extent_.init(x0 - padding, y0 - padding, x1 + padding , y1 + padding);
    }

    boost::optional<box2d<double> > const& maximum_extent = lay.maximum_extent();
    if (maximum_extent)
    {
        query_extent_.clip(*maximum_extent);
    }
}

template <typename T>
void grid_renderer<T>::end_layer_processing(layer const&)
{
    MAPNIK_LOG_DEBUG(grid_renderer) << "grid_renderer: End layer processing";
}

template <typename T>
void grid_renderer<T>::render_marker(mapnik::feature_impl & feature, unsigned int step, pixel_position const& pos, marker const& marker, agg::trans_affine const& tr, double opacity, composite_mode_e comp_op)
{
    if (marker.is_vector())
    {
        typedef coord_transform<CoordTransform,geometry_type> path_type;
        typedef agg::renderer_base<mapnik::pixfmt_gray32> ren_base;
        typedef agg::renderer_scanline_bin_solid<ren_base> renderer;
        agg::scanline_bin sl;

        grid_rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_);
        mapnik::pixfmt_gray32 pixf(buf);

        ren_base renb(pixf);
        renderer ren(renb);

        ras_ptr->reset();

        box2d<double> const& bbox = (*marker.get_vector_data())->bounding_box();
        coord<double,2> c = bbox.center();
        // center the svg marker on '0,0'
        agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
        // apply symbol transformation to get to map space
        mtx *= tr;
        mtx *= agg::trans_affine_scaling(scale_factor_*(1.0/step));
        // render the marker at the center of the marker box
        mtx.translate(pos.x, pos.y);
        using namespace mapnik::svg;
        vertex_stl_adapter<svg_path_storage> stl_storage((*marker.get_vector_data())->source());
        svg_path_adapter svg_path(stl_storage);
        svg_renderer_agg<svg_path_adapter,
            agg::pod_bvector<path_attributes>,
            renderer,
            mapnik::pixfmt_gray32> svg_renderer(svg_path,
                                                (*marker.get_vector_data())->attributes());

        svg_renderer.render_id(*ras_ptr, sl, renb, feature.id(), mtx, opacity, bbox);

    }
    else
    {
        image_data_32 const& data = **marker.get_bitmap_data();
        double width = data.width();
        double height = data.height();
        double cx = 0.5 * width;
        double cy = 0.5 * height;
        if (step == 1 && (std::fabs(1.0 - scale_factor_) < 0.001 && tr.is_identity()))
        {
            // TODO - support opacity
            pixmap_.set_rectangle(feature.id(), data,
                                  boost::math::iround(pos.x - cx),
                                  boost::math::iround(pos.y - cy));
        }
        else
        {
            // TODO - remove support for step != or add support for agg scaling with opacity
            double ratio = (1.0/step);
            image_data_32 target(ratio * data.width(), ratio * data.height());
            mapnik::scale_image_agg<image_data_32>(target,data, SCALING_NEAR,
                                                   scale_factor_, 0.0, 0.0, 1.0, ratio);
            pixmap_.set_rectangle(feature.id(), target,
                                  boost::math::iround(pos.x - cx),
                                  boost::math::iround(pos.y - cy));
        }
    }
    pixmap_.add_feature(feature);
}

template class grid_renderer<grid>;

}
