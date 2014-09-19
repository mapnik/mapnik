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

#if defined(HAVE_CAIRO)

#ifndef MAPNIK_CAIRO_RENDERER_HPP
#define MAPNIK_CAIRO_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/map.hpp>
#include <mapnik/request.hpp>
#include <mapnik/rule.hpp> // for all symbolizers
#include <mapnik/noncopyable.hpp>
#include <mapnik/cairo_context.hpp>
#include <mapnik/pixel_position.hpp>

// cairo
#include <cairo.h>

// boost
#include <boost/scoped_ptr.hpp>

namespace agg {
struct trans_affine;
}

namespace mapnik {

class marker;

class MAPNIK_DECL cairo_renderer_base : private mapnik::noncopyable
{
protected:
    cairo_renderer_base(Map const& m,
                        cairo_ptr const& cairo,
                        double scale_factor=1.0,
                        unsigned offset_x=0,
                        unsigned offset_y=0);
    cairo_renderer_base(Map const& m,
                        request const& req,
                        cairo_ptr const& cairo,
                        double scale_factor=1.0,
                        unsigned offset_x=0,
                        unsigned offset_y=0);
    cairo_renderer_base(Map const& m,
                        cairo_ptr const& cairo,
                        boost::shared_ptr<label_collision_detector4> detector,
                        double scale_factor=1.0,
                        unsigned offset_x=0,
                        unsigned offset_y=0);
public:
    ~cairo_renderer_base();
    void start_map_processing(Map const& map);
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
        return scale_factor_;
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
    unsigned width_;
    unsigned height_;
    double scale_factor_;
    CoordTransform t_;
    boost::shared_ptr<freetype_engine> font_engine_;
    face_manager<freetype_engine> font_manager_;
    cairo_face_manager face_manager_;
    boost::shared_ptr<label_collision_detector4> detector_;
    box2d<double> query_extent_;
    void setup(Map const& m);
};

template <typename T>
class MAPNIK_DECL cairo_renderer : public feature_style_processor<cairo_renderer<T> >,
                                   public cairo_renderer_base
{
public:
    typedef cairo_renderer_base processor_impl_type;
    cairo_renderer(Map const& m,
                   T const& obj,
                   double scale_factor=1.0,
                   unsigned offset_x=0,
                   unsigned offset_y=0);
    cairo_renderer(Map const& m,
                   request const& req,
                   T const& obj,
                   double scale_factor=1.0,
                   unsigned offset_x=0,
                   unsigned offset_y=0);
    cairo_renderer(Map const& m,
                   T const& obj,
                   boost::shared_ptr<label_collision_detector4> detector,
                   double scale_factor=1.0,
                   unsigned offset_x=0,
                   unsigned offset_y=0);
    void end_map_processing(Map const& map);
};
}

#endif // MAPNIK_CAIRO_RENDERER_HPP

#endif
