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

#ifndef MAPNIK_METAWRITER_RENDERER_HPP
#define MAPNIK_METAWRITER_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/map.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

namespace agg {
struct trans_affine;
}

namespace mapnik {

class MAPNIK_DECL metawriter_renderer : public feature_style_processor<metawriter_renderer>,
                                        private boost::noncopyable
{

public:
    typedef metawriter_renderer processor_impl_type;
    metawriter_renderer(Map const& m, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    metawriter_renderer(Map const &m, boost::shared_ptr<label_collision_detector4> detector,
                 double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    ~metawriter_renderer();
    void start_map_processing(Map const& map);
    void end_map_processing(Map const& map);
    void start_layer_processing(layer const& lay, box2d<double> const& query_extent);
    void end_layer_processing(layer const& lay);

    void start_style_processing(feature_type_style const& st);
    void end_style_processing(feature_type_style const& st);
    
    /*
    void process(point_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    */
    void process(line_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void painted(bool painted);
private:
    unsigned width_;
    unsigned height_;
    double scale_factor_;
    CoordTransform t_;
    freetype_engine font_engine_;
    face_manager<freetype_engine> font_manager_;
    boost::shared_ptr<label_collision_detector4> detector_;
    box2d<double> query_extent_;
    void setup(Map const &m);
};
}

#endif // MAPNIK_METAWRITER_RENDERER_HPP
