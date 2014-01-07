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

#ifndef MAPNIK_AGG_RENDERER_HPP
#define MAPNIK_AGG_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp>            // for MAPNIK_DECL
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/font_engine_freetype.hpp>  // for face_manager, etc
#include <mapnik/noncopyable.hpp>       // for noncopyable
#include <mapnik/rule.hpp>              // for rule, symbolizers
#include <mapnik/box2d.hpp>     // for box2d
#include <mapnik/color.hpp>     // for color
#include <mapnik/ctrans.hpp>    // for CoordTransform
#include <mapnik/image_compositing.hpp>  // for composite_mode_e
#include <mapnik/pixel_position.hpp>
#include <mapnik/request.hpp>
#include <mapnik/gamma_method.hpp>
#include <mapnik/renderer_common.hpp>
// boost

#include <memory>

// fwd declaration to avoid depedence on agg headers
namespace agg { struct trans_affine; }

// fwd declarations to speed up compile
namespace mapnik {
  class Map;
  class feature_impl;
  class feature_type_style;
  class label_collision_detector4;
  class layer;
  class marker;
  class proj_transform;
  struct rasterizer;
}

namespace mapnik {

template <typename T0, typename T1=label_collision_detector4>
class MAPNIK_DECL agg_renderer : public feature_style_processor<agg_renderer<T0> >,
                                 private mapnik::noncopyable
{

public:
    typedef T0 buffer_type;
    typedef agg_renderer<T0> processor_impl_type;
    typedef T1 detector_type;
    // create with default, empty placement detector
    agg_renderer(Map const& m, buffer_type & pixmap, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    // create with external placement detector, possibly non-empty
    agg_renderer(Map const &m, buffer_type & pixmap, std::shared_ptr<detector_type> detector,
                 double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    // pass in mapnik::request object to provide the mutable things per render
    agg_renderer(Map const& m, request const& req, buffer_type & pixmap, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    ~agg_renderer();
    void start_map_processing(Map const& map);
    void end_map_processing(Map const& map);
    void start_layer_processing(layer const& lay, box2d<double> const& query_extent);
    void end_layer_processing(layer const& lay);

    void start_style_processing(feature_type_style const& st);
    void end_style_processing(feature_type_style const& st);

    void render_marker(pixel_position const& pos, marker const& marker, agg::trans_affine const& tr,
                       double opacity, composite_mode_e comp_op);

    void process(point_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(line_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(line_pattern_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(polygon_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(polygon_pattern_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(raster_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(shield_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(text_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(building_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(markers_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(debug_symbolizer const& sym,
                 feature_impl & feature,
                 proj_transform const& prj_trans);

    inline bool process(rule::symbolizers const&,
                        mapnik::feature_impl&,
                        proj_transform const& )
    {
        // agg renderer doesn't support processing of multiple symbolizers.
        return false;
    }

    void painted(bool painted);
    inline eAttributeCollectionPolicy attribute_collection_policy() const
    {
        return DEFAULT;
    }

    inline double scale_factor() const
    {
        return common_.scale_factor_;
    }

    inline box2d<double> clipping_extent() const
    {
        if (common_.t_.offset() > 0)
        {
            box2d<double> box = common_.query_extent_;
            double scale = static_cast<double>(common_.query_extent_.width())/static_cast<double>(common_.width_);
            // 3 is used here because at least 3 was needed for the 'style-level-compositing-tiled-0,1' visual test to pass
            // TODO - add more tests to hone in on a more robust #
            scale *= common_.t_.offset()*3;
            box.pad(scale);
            return box;
        }
        return common_.query_extent_;
    }

protected:
    template <typename R>
    void debug_draw_box(R& buf, box2d<double> const& extent,
                        double x, double y, double angle = 0.0);
    void debug_draw_box(box2d<double> const& extent,
                        double x, double y, double angle = 0.0);
    void draw_geo_extent(box2d<double> const& extent,mapnik::color const& color);

private:
    buffer_type & pixmap_;
    std::shared_ptr<buffer_type> internal_buffer_;
    mutable buffer_type * current_buffer_;
    mutable bool style_level_compositing_;
    const std::unique_ptr<rasterizer> ras_ptr;
    gamma_method_enum gamma_method_;
    double gamma_;
    renderer_common common_;
    void setup(Map const& m);
};
}

#endif // MAPNIK_AGG_RENDERER_HPP
