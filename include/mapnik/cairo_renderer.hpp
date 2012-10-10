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
#include <mapnik/rule.hpp> // for all symbolizers

// cairo
#include <cairomm/context.h>
#include <cairomm/surface.h>

// boost
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

// FIXME
// forward declare so that
// apps using mapnik do not
// need agg headers
namespace agg {
struct trans_affine;
}

namespace mapnik {

class marker;

class cairo_face;

typedef boost::shared_ptr<cairo_face> cairo_face_ptr;

class cairo_face_manager : private boost::noncopyable
{
public:
    cairo_face_manager(boost::shared_ptr<freetype_engine> engine);
    cairo_face_ptr get_face(face_ptr face);

private:
    typedef std::map<face_ptr,cairo_face_ptr> cairo_face_cache;
    boost::shared_ptr<freetype_engine> font_engine_;
    cairo_face_cache cache_;
};

class MAPNIK_DECL cairo_renderer_base : private boost::noncopyable
{
protected:
    cairo_renderer_base(Map const& m, Cairo::RefPtr<Cairo::Context> const& context, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    cairo_renderer_base(Map const& m, Cairo::RefPtr<Cairo::Context> const& context, boost::shared_ptr<label_collision_detector4> detector, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
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
    };
    void painted(bool /*painted*/)
    {
        // nothing to do
    }

    void render_marker(pixel_position const& pos, marker const& marker, const agg::trans_affine & mtx, double opacity=1.0, bool recenter=true);
    void render_box(box2d<double> const& b);
protected:


    Map const& m_;
    Cairo::RefPtr<Cairo::Context> context_;
    unsigned width_;
    unsigned height_;
    double scale_factor_;
    CoordTransform t_;
    boost::shared_ptr<freetype_engine> font_engine_;
    face_manager<freetype_engine> font_manager_;
    cairo_face_manager face_manager_;
    boost::shared_ptr<label_collision_detector4> detector_;
    box2d<double> query_extent_;
};

template <typename T>
class MAPNIK_DECL cairo_renderer : public feature_style_processor<cairo_renderer<T> >,
                                   public cairo_renderer_base
{
public:
    typedef cairo_renderer_base processor_impl_type;
    cairo_renderer(Map const& m, Cairo::RefPtr<T> const& surface, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    cairo_renderer(Map const& m, Cairo::RefPtr<T> const& surface, boost::shared_ptr<label_collision_detector4> detector, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    void end_map_processing(Map const& map);
};
}

#endif // MAPNIK_CAIRO_RENDERER_HPP

#endif
