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

// mapnik
#include <mapnik/graphics.hpp>
#include <mapnik/metawriter_renderer.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/map.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/make_shared.hpp>
#include <boost/math/special_functions/round.hpp>

// stl
#include <cmath>

namespace mapnik
{

metawriter_renderer::metawriter_renderer(Map const& m,double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<metawriter_renderer>(m, scale_factor),
      width_(m.width()),
      height_(m.height()),
      scale_factor_(scale_factor),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(),
      font_manager_(font_engine_),
      detector_(boost::make_shared<label_collision_detector4>(box2d<double>(-m.buffer_size(), -m.buffer_size(), m.width() + m.buffer_size() ,m.height() + m.buffer_size())))
{
    setup(m);
}

metawriter_renderer::metawriter_renderer(Map const& m, boost::shared_ptr<label_collision_detector4> detector,
                              double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<metawriter_renderer>(m, scale_factor),
      width_(m.width()),
      height_(m.height()),
      scale_factor_(scale_factor),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(),
      font_manager_(font_engine_),
      detector_(detector)
{
    setup(m);
}

void metawriter_renderer::setup(Map const &m)
{
    MAPNIK_LOG_DEBUG(metawriter_renderer) << "metawriter_renderer: Scale=" << m.scale();
}

metawriter_renderer::~metawriter_renderer() {}

void metawriter_renderer::start_map_processing(Map const& m)
{
    MAPNIK_LOG_DEBUG(metawriter_renderer) << "metawriter_renderer: Start map processing bbox=" << m.get_current_extent();

    projection proj(m.srs());
    Map::const_metawriter_iterator metaItr = m.begin_metawriters();
    Map::const_metawriter_iterator metaItrEnd = m.end_metawriters();
    for (;metaItr!=metaItrEnd; ++metaItr)
    {
        set_size(metaItr->second, m.width(), m.height());
        set_map_srs(metaItr->second, proj);
        start(metaItr->second, m.metawriter_output_properties);
    }
}

void metawriter_renderer::end_map_processing(Map const& m)
{
    Map::const_metawriter_iterator metaItr = m.begin_metawriters();
    Map::const_metawriter_iterator metaItrEnd = m.end_metawriters();
    for (;metaItr!=metaItrEnd; ++metaItr)
    {
        stop(metaItr->second);
    }
    MAPNIK_LOG_DEBUG(metawriter_renderer) << "metawriter_renderer: End map processing";
}

void metawriter_renderer::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(metawriter_renderer) << "metawriter_renderer: Start processing layer=" << lay.name();
    MAPNIK_LOG_DEBUG(metawriter_renderer) << "metawriter_renderer: -- datasource=" << lay.datasource().get();
    MAPNIK_LOG_DEBUG(metawriter_renderer) << "metawriter_renderer: -- query_extent=" << query_extent;

    if (lay.clear_label_cache())
    {
        detector_->clear();
    }
    query_extent_ = query_extent;
}

void metawriter_renderer::end_layer_processing(layer const&)
{
    MAPNIK_LOG_DEBUG(metawriter_renderer) << "metawriter_renderer: End layer processing";
}

void metawriter_renderer::start_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(metawriter_renderer) << "metawriter_renderer: Start processing style";
}

void metawriter_renderer::end_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(metawriter_renderer) << "metawriter_renderer: End processing style";
}

void metawriter_renderer::painted(bool painted)
{
    //
}


}
