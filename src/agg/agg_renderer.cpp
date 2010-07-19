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

// mapnik
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_cache.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/text_path.hpp>

// agg
#define AGG_RENDERING_BUFFER row_ptr_cache<int8u>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_basics.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_path_storage.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_contour.h"
#include "agg_conv_clip_polyline.h"
#include "agg_vcgen_stroke.h"
#include "agg_conv_adaptor_vcgen.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_conv_marker.h"
#include "agg_vcgen_markers_term.h"
#include "agg_renderer_outline_aa.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_rasterizer_outline.h"
#include "agg_renderer_outline_image.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_pattern_filters_rgba.h"
#include "agg_renderer_outline_image.h"
#include "agg_vpgen_clip_polyline.h"
#include "agg_arrowhead.h"

// boost
#include <boost/utility.hpp>


// stl
#ifdef MAPNIK_DEBUG
#include <iostream>
#endif

#include <cmath>

namespace mapnik
{
class pattern_source : private boost::noncopyable
{
public:
    pattern_source(image_data_32 const& pattern)
        : pattern_(pattern) {}

    unsigned int width() const
    {
        return pattern_.width();
    }
    unsigned int height() const
    {
        return pattern_.height();
    }
    agg::rgba8 pixel(int x, int y) const
    {
        unsigned c = pattern_(x,y);
        return agg::rgba8(c & 0xff,
                          (c >> 8) & 0xff,
                          (c >> 16) & 0xff,
                          (c >> 24) & 0xff);
    }
private:
    image_data_32 const& pattern_;
};


template <typename T>
agg_renderer<T>::agg_renderer(Map const& m, T & pixmap, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<agg_renderer>(m, scale_factor),
      pixmap_(pixmap),
      width_(pixmap_.width()),
      height_(pixmap_.height()),
      scale_factor_(scale_factor),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(),
      font_manager_(font_engine_),
      detector_(box2d<double>(-m.buffer_size(), -m.buffer_size(), m.width() + m.buffer_size() ,m.height() + m.buffer_size())),
      ras_ptr(new rasterizer)
{
    boost::optional<color> const& bg = m.background();
    if (bg) pixmap_.set_background(*bg);
    
    boost::optional<std::string> const& image_filename = m.background_image();
    if (image_filename)
    {
        boost::optional<mapnik::image_ptr> bg_image = mapnik::image_cache::instance()->find(*image_filename,true);
        if (bg_image)
        {
            int w = (*bg_image)->width();
            int h = (*bg_image)->height();
            if ( w > 0 && h > 0)
            {
                // repeat background-image both vertically and horizontally
                unsigned x_steps = unsigned(std::ceil(width_/double(w)));
                unsigned y_steps = unsigned(std::ceil(height_/double(h)));
                for (unsigned x=0;x<x_steps;++x)
                {
                    for (unsigned y=0;y<y_steps;++y)
                    {
                        pixmap_.set_rectangle_alpha2(*(*bg_image), x*w, y*h, 1.0f);
                    }
                }
            }
        }
    }
#ifdef MAPNIK_DEBUG
    std::clog << "scale=" << m.scale() << "\n";
#endif
}

template <typename T>
agg_renderer<T>::~agg_renderer() {}

template <typename T>
void agg_renderer<T>::start_map_processing(Map const& map)
{
#ifdef MAPNIK_DEBUG
    std::clog << "start map processing bbox="
              << map.get_current_extent() << "\n";
#endif
    ras_ptr->clip_box(0,0,width_,height_);
}

template <typename T>
void agg_renderer<T>::end_map_processing(Map const& )
{
#ifdef MAPNIK_DEBUG
    std::clog << "end map processing\n";
#endif
}

template <typename T>
void agg_renderer<T>::start_layer_processing(layer const& lay)
{
#ifdef MAPNIK_DEBUG
    std::clog << "start layer processing : " << lay.name()  << "\n";
    std::clog << "datasource = " << lay.datasource().get() << "\n";
#endif
    if (lay.clear_label_cache())
    {
        detector_.clear();
    }
}

template <typename T>
void agg_renderer<T>::end_layer_processing(layer const&)
{
#ifdef MAPNIK_DEBUG
    std::clog << "end layer processing\n";
#endif
}

template class agg_renderer<image_32>;
}
