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

#ifndef GRID_RENDERER_HPP
#define GRID_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/map.hpp>
//#include <mapnik/marker.hpp>

#include <mapnik/grid/grid.hpp>

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
   
struct grid_rasterizer;
   
template <typename T>
class MAPNIK_DECL grid_renderer : public feature_style_processor<grid_renderer<T> >,
                                  private boost::noncopyable
{
     
public:
    grid_renderer(Map const& m, T & pixmap, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    ~grid_renderer();
    void start_map_processing(Map const& map);
    void end_map_processing(Map const& map);
    void start_layer_processing(layer const& lay);
    void end_layer_processing(layer const& lay);
    void render_marker(Feature const& feature, unsigned int step, const int x, const int y, marker &marker, const agg::trans_affine & tr, double opacity);

    void process(point_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(line_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(line_pattern_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(polygon_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(polygon_pattern_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(raster_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(shield_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(text_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(building_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(markers_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);
    void process(glyph_symbolizer const& sym,
                 Feature const& feature,
                 proj_transform const& prj_trans);    
    inline bool process(rule::symbolizers const& /*syms*/,
                        Feature const& /*feature*/,
                        proj_transform const& /*prj_trans*/)
    {
        // grid renderer doesn't support processing of multiple symbolizers.
        return false;
    };
    void painted(bool painted)
    {
        pixmap_.painted(painted);
    }

private:
    T & pixmap_;
    unsigned width_;
    unsigned height_;
    double scale_factor_;
    CoordTransform t_;
    freetype_engine font_engine_;
    face_manager<freetype_engine> font_manager_;
    label_collision_detector4 detector_;
    boost::scoped_ptr<grid_rasterizer> ras_ptr;
};
}

#endif //GRID_RENDERER_HPP
