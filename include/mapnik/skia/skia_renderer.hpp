/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#if defined(HAVE_SKIA)

#ifndef MAPNIK_SKIA_RENDERER_HPP
#define MAPNIK_SKIA_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/skia/skia_font_manager.hpp>
#include <mapnik/skia/skia_typeface_cache.hpp>
#include <mapnik/label_collision_detector.hpp>

// skia fwd decl
class SkCanvas;

namespace mapnik
{

class MAPNIK_DECL skia_renderer : public feature_style_processor<skia_renderer>,
                                  private mapnik::noncopyable
{

public:
    typedef skia_renderer processor_impl_type;

    skia_renderer(Map const& map, SkCanvas & canvas, double scale_factor = 1.0);
    ~skia_renderer();

    void start_map_processing(Map const& map);
    void end_map_processing(Map const& map);
    void start_layer_processing(layer const& lay, box2d<double> const& query_extent);
    void end_layer_processing(layer const& lay);
    void start_style_processing(feature_type_style const& st);
    void end_style_processing(feature_type_style const& st);
    void process(line_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);

    void process(polygon_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);

    void process(text_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans);

    void painted(bool painted) {};

    inline bool process(rule::symbolizers const& /*syms*/,
                        mapnik::feature_impl & /*feature*/,
                        proj_transform const& /*prj_trans*/)
    {
        return false;
    }

    inline eAttributeCollectionPolicy attribute_collection_policy() const
    {
        return DEFAULT;
    }

private:
    SkCanvas & canvas_;
    unsigned width_;
    unsigned height_;
    CoordTransform t_;
    double scale_factor_;
    box2d<double> query_extent_;
    skia_font_manager font_manager_;
    skia_typeface_cache typeface_cache_;
    boost::shared_ptr<label_collision_detector4> detector_;
};

}


#endif //MAPNIK_SKIA_RENDERER_HPP
#endif // HAVE_SKIA
