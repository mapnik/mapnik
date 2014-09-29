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

#if defined(GRID_RENDERER)

// mapnik
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>


#include <mapnik/image_scaling.hpp>
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
#include <mapnik/request.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/pixel_position.hpp>

// boost
#include <boost/math/special_functions/round.hpp>

// agg
#include "agg_trans_affine.h"

namespace mapnik
{

template <typename T>
grid_renderer<T>::grid_renderer(Map const& m, T & pixmap, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<grid_renderer>(m, scale_factor),
      pixmap_(pixmap),
      ras_ptr(new grid_rasterizer),
      common_(m, attributes(), offset_x, offset_y, m.width(), m.height(), scale_factor)
{
    setup(m);
}

template <typename T>
grid_renderer<T>::grid_renderer(Map const& m, request const& req, attributes const& vars, T & pixmap, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<grid_renderer>(m, scale_factor),
      pixmap_(pixmap),
      ras_ptr(new grid_rasterizer),
      common_(m, req, vars, offset_x, offset_y, req.width(), req.height(), scale_factor)
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

    ras_ptr->clip_box(0,0,common_.width_,common_.height_);
}

template <typename T>
void grid_renderer<T>::end_map_processing(Map const& /*m*/)
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
        common_.detector_->clear();
    }
    common_.query_extent_ = query_extent;
    boost::optional<box2d<double> > const& maximum_extent = lay.maximum_extent();
    if (maximum_extent)
    {
        common_.query_extent_.clip(*maximum_extent);
    }
}

template <typename T>
void grid_renderer<T>::end_layer_processing(layer const&)
{
    MAPNIK_LOG_DEBUG(grid_renderer) << "grid_renderer: End layer processing";
}

template <typename T>
void grid_renderer<T>::render_marker(mapnik::feature_impl const& feature, pixel_position const& pos, marker const& marker, agg::trans_affine const& tr, double opacity, composite_mode_e /*comp_op*/)
{
    if (marker.is_vector())
    {
        using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
        using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;
        agg::scanline_bin sl;

        grid_rendering_buffer buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
        pixfmt_type pixf(buf);

        grid_renderer_base_type renb(pixf);
        renderer_type ren(renb);

        ras_ptr->reset();

        box2d<double> const& bbox = (*marker.get_vector_data())->bounding_box();
        coord<double,2> c = bbox.center();
        // center the svg marker on '0,0'
        agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
        // apply symbol transformation to get to map space
        mtx *= tr;
        mtx *= agg::trans_affine_scaling(common_.scale_factor_);
        // render the marker at the center of the marker box
        mtx.translate(pos.x, pos.y);
        using namespace mapnik::svg;
        vertex_stl_adapter<svg_path_storage> stl_storage((*marker.get_vector_data())->source());
        svg_path_adapter svg_path(stl_storage);
        svg_renderer_agg<svg_path_adapter,
            agg::pod_bvector<path_attributes>,
            renderer_type,
            pixfmt_type> svg_renderer(svg_path,
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
        if ((std::fabs(1.0 - common_.scale_factor_) < 0.001 && tr.is_identity()))
        {
            // TODO - support opacity
            pixmap_.set_rectangle(feature.id(), data,
                                  boost::math::iround(pos.x - cx),
                                  boost::math::iround(pos.y - cy));
        }
        else
        {
            image_data_32 target(data.width(), data.height());
            mapnik::scale_image_agg<image_data_32>(target,
                                                   data,
                                                   SCALING_NEAR,
                                                   1,
                                                   1,
                                                   0.0, 0.0, 1.0); // TODO: is 1.0 a valid default here, and do we even care in grid_renderer what the image looks like?
            pixmap_.set_rectangle(feature.id(), target,
                                  boost::math::iround(pos.x - cx),
                                  boost::math::iround(pos.y - cy));
        }
    }
    pixmap_.add_feature(feature);
}

template class grid_renderer<grid>;

}

#endif
