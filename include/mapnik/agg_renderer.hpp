/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

#ifndef AGG_RENDERER_HPP
#define AGG_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/map.hpp>
// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
// boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace mapnik {
    template <typename T>
    class MAPNIK_DECL agg_renderer : public feature_style_processor<agg_renderer<T> >,
                                     private boost::noncopyable
    {
    public:
        agg_renderer(Map const& m, T & pixmap, unsigned offset_x=0, unsigned offset_y=0);
        void start_map_processing(Map const& map);
        void end_map_processing(Map const& map);
        void start_layer_processing(Layer const& lay);
        void end_layer_processing(Layer const& lay);
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
    private:
          T & pixmap_;
          unsigned width_;
          unsigned height_;
          agg::rendering_buffer buf_;
          agg::pixfmt_rgba32 pixf_;
          CoordTransform t_;
          freetype_engine font_engine_;
          face_manager<freetype_engine> font_manager_;
          label_collision_detector4 detector_;
          placement_finder<label_collision_detector4> finder_;
          agg::rasterizer_scanline_aa<> ras_;
    };
}

#endif //AGG_RENDERER_HPP
