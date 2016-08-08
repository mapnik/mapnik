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

#if defined(HAVE_CAIRO)

#ifndef MAPNIK_CAIRO_RENDERER_HPP
#define MAPNIK_CAIRO_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/rule.hpp> // for all symbolizers
#include <mapnik/cairo/cairo_context.hpp>
#include <mapnik/renderer_common.hpp>

// stl
#include <memory>

namespace agg {
struct trans_affine;
}

namespace mapnik {

class Map;
class feature_impl;
class feature_type_style;
class label_collision_detector4;
class layer;
struct marker;
class proj_transform;
class request;
struct pixel_position;
struct cairo_save_restore
{
    cairo_save_restore(cairo_context & context)
        : context_(context)
    {
        context_.save();
    }
    ~cairo_save_restore()
    {
        context_.restore();
    }
    cairo_context & context_;
};

template <typename T>
class MAPNIK_DECL cairo_renderer : public feature_style_processor<cairo_renderer<T> >,
                                   private util::noncopyable
{
public:
    using processor_impl_type = cairo_renderer<T>;
    cairo_renderer(Map const& m,
                   T const& obj,
                   double scale_factor=1.0,
                   unsigned offset_x=0,
                   unsigned offset_y=0);
    cairo_renderer(Map const& m,
                   request const& req,
                   attributes const& vars,
                   T const& obj,
                   double scale_factor=1.0,
                   unsigned offset_x=0,
                   unsigned offset_y=0);
    cairo_renderer(Map const& m,
                   T const& obj,
                   std::shared_ptr<label_collision_detector4> detector,
                   double scale_factor=1.0,
                   unsigned offset_x=0,
                   unsigned offset_y=0);

    ~cairo_renderer();
    void start_map_processing(Map const& map);
    void end_map_processing(Map const& map);
    void start_layer_processing(layer const& lay, box2d<double> const& query_extent);
    void end_layer_processing(layer const& lay);
    void start_style_processing(feature_type_style const& st);
    void end_style_processing(feature_type_style const& st);
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
    void process(group_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    void process(debug_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);
    inline bool process(rule::symbolizers const& /*syms*/,
                        mapnik::feature_impl & /*feature*/,
                        proj_transform const& /*prj_trans*/)
    {
        // cairo renderer doesn't support processing of multiple symbolizers.
        return false;
    }

    bool painted()
    {
        return true;
    }

    void painted(bool /*painted*/)
    {
        // nothing to do
    }

    inline eAttributeCollectionPolicy attribute_collection_policy() const
    {
        return DEFAULT;
    }

    inline double scale_factor() const
    {
        return common_.scale_factor_;
    }

    inline attributes const& variables() const
    {
        return common_.vars_;
    }

    void render_marker(pixel_position const& pos,
                       marker const& marker,
                       agg::trans_affine const& mtx,
                       double opacity=1.0,
                       bool recenter=true);
    void render_box(box2d<double> const& b);
protected:
    Map const& m_;
    cairo_context context_;
    renderer_common common_;
    cairo_face_manager face_manager_;
    bool style_level_compositing_;
    void setup(Map const& m);

};

extern template class MAPNIK_DECL cairo_renderer<cairo_ptr>;

}

#endif // MAPNIK_CAIRO_RENDERER_HPP

#endif
