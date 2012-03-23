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
#include <mapnik/config.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/map.hpp>

// agg FIXME 

#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"
#include "agg_scanline_p.h"
//#include "agg_renderer_outline_aa.h"
#include "agg_renderer_scanline.h"
// boost
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

// FIXME
// forward declare so that
// apps using mapnik do not
// need agg headers
namespace agg {
struct trans_affine;
}


namespace mapnik {

class marker;

struct rasterizer;


template <typename PixelFormat>
struct stroke_renderer
{
    typedef PixelFormat pixfmt_type;
    typedef typename pixfmt_type::color_type color_type;
    typedef typename pixfmt_type::row_data row_data;
    typedef agg::renderer_base<pixfmt_type> ren_base;  
    typedef agg::renderer_scanline_aa_solid<ren_base> renderer;
    typedef agg::scanline_u8 scanline_type;
    
    stroke_renderer()
        : renb_(),
          ren_(renb_)
    {}
    
    template <typename PF>
    void attach(PF & pf)
    {
        renb_.attach(pf);
    }
    
    void color(color_type const& c)
    {
        ren_.color(c);
    }
    
    template <typename Rasterizer>
    void render(Rasterizer & ras)
    {
        agg::render_scanlines(ras, sl_, ren_);
    }

    scanline_type sl_;
    ren_base renb_;
    renderer ren_;
};


template <typename T>
class MAPNIK_DECL agg_renderer : public feature_style_processor<agg_renderer<T> >,
                                 private boost::noncopyable
{

public:
    // create with default, empty placement detector
    agg_renderer(Map const& m, T & pixmap, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    // create with external placement detector, possibly non-empty
    agg_renderer(Map const &m, T & pixmap, boost::shared_ptr<label_collision_detector4> detector,
                 double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    ~agg_renderer();
    void start_map_processing(Map const& map);
    void end_map_processing(Map const& map);
    void start_layer_processing(layer const& lay, box2d<double> const& query_extent);
    void end_layer_processing(layer const& lay);
    void render_marker(pixel_position const& pos, marker const& marker, agg::trans_affine const& tr, double opacity);

    void process(point_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(line_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(line_pattern_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(polygon_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(polygon_pattern_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(raster_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(shield_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(text_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(building_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(markers_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);
    void process(compositing_symbolizer const& sym,
                 mapnik::feature_ptr const& feature,
                 proj_transform const& prj_trans);

    inline bool process(rule::symbolizers const& /*syms*/,
                        mapnik::feature_ptr const& /*feature*/,
                        proj_transform const& /*prj_trans*/)
    {
        // agg renderer doesn't support processing of multiple symbolizers.
        return false;
    };
    void painted(bool painted)
    {
        pixmap_.painted(painted);
    }

private:
    T & pixmap_;
    stroke_renderer<agg::pixfmt_rgba32_plain> stroker_;
    unsigned width_;
    unsigned height_;
    double scale_factor_;
    CoordTransform t_;
    freetype_engine font_engine_;
    face_manager<freetype_engine> font_manager_;
    boost::shared_ptr<label_collision_detector4> detector_;
    boost::scoped_ptr<rasterizer> ras_ptr;
    box2d<double> query_extent_;
    void setup(Map const &m);
};
}

#endif // MAPNIK_AGG_RENDERER_HPP
