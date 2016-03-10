/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/math/special_functions/round.hpp>
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_trans_affine.h"
#pragma GCC diagnostic pop

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

template <typename buffer_type>
struct grid_render_marker_visitor
{
    grid_render_marker_visitor(buffer_type & pixmap,
                               std::unique_ptr<grid_rasterizer> const& ras_ptr,
                               renderer_common const& common,
                               mapnik::feature_impl const& feature,
                               pixel_position const& pos,
                               agg::trans_affine const& tr,
                               double opacity)
        : pixmap_(pixmap),
          ras_ptr_(ras_ptr),
          common_(common),
          feature_(feature),
          pos_(pos),
          tr_(tr),
          opacity_(opacity) {}

    void operator() (marker_null const&) {}

    void operator() (marker_svg const& marker)
    {
        using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
        using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;
        agg::scanline_bin sl;

        grid_rendering_buffer buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
        pixfmt_type pixf(buf);

        grid_renderer_base_type renb(pixf);
        renderer_type ren(renb);

        ras_ptr_->reset();

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
            agg::pod_bvector<path_attributes>,
            renderer_type,
            pixfmt_type> svg_renderer(svg_path,
                                                marker.get_data()->attributes());

        svg_renderer.render_id(*ras_ptr_, sl, renb, feature_.id(), mtx, opacity_, bbox);
    }

    void operator() (marker_rgba8 const& marker)
    {
        image_rgba8 const& data = marker.get_data();
        double width = data.width();
        double height = data.height();
        double cx = 0.5 * width;
        double cy = 0.5 * height;
        if ((std::fabs(1.0 - common_.scale_factor_) < 0.001 && tr_.is_identity()))
        {
            // TODO - support opacity
            pixmap_.set_rectangle(feature_.id(), data,
                                  boost::math::iround(pos_.x - cx),
                                  boost::math::iround(pos_.y - cy));
        }
        else
        {
            image_rgba8 target(data.width(), data.height());
            boost::optional<double> nodata;
            mapnik::scale_image_agg(target,
                                    data,
                                    SCALING_NEAR,
                                    1,
                                    1,
                                    0.0, 0.0, 1.0, nodata); // TODO: is 1.0 a valid default here, and do we even care in grid_renderer what the image looks like?
            pixmap_.set_rectangle(feature_.id(), target,
                                  boost::math::iround(pos_.x - cx),
                                  boost::math::iround(pos_.y - cy));
        }
    }

  private:
    buffer_type & pixmap_;
    std::unique_ptr<grid_rasterizer> const& ras_ptr_;
    renderer_common const& common_;
    mapnik::feature_impl const& feature_;
    pixel_position const& pos_;
    agg::trans_affine const& tr_;
    double opacity_;
};

template <typename T>
void grid_renderer<T>::render_marker(mapnik::feature_impl const& feature,
                                     pixel_position const& pos,
                                     marker const& marker,
                                     agg::trans_affine const& tr,
                                     double opacity,
                                     composite_mode_e /*comp_op*/)
{
    grid_render_marker_visitor<buffer_type> visitor(pixmap_,
                                ras_ptr,
                                common_,
                                feature,
                                pos,
                                tr,
                                opacity);
    util::apply_visitor(visitor, marker);
    pixmap_.add_feature(feature);
}

template class MAPNIK_DECL grid_renderer<grid>;

}

#endif
