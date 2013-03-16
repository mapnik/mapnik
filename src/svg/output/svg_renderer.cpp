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
#include <mapnik/debug.hpp>
#include <mapnik/svg/output/svg_renderer.hpp>
#include <mapnik/map.hpp>

// stl
#include <ostream>

namespace mapnik
{
template <typename T>
svg_renderer<T>::svg_renderer(Map const& m, T & output_iterator, unsigned offset_x, unsigned offset_y) :
    feature_style_processor<svg_renderer>(m),
    output_iterator_(output_iterator),
    width_(m.width()),
    height_(m.height()),
    t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
    generator_(output_iterator)
{}

template <typename T>
svg_renderer<T>::~svg_renderer() {}

template <typename T>
void svg_renderer<T>::start_map_processing(Map const& map)
{
    MAPNIK_LOG_DEBUG(svg_renderer) << "svg_renderer: Start map processing";

    // generate XML header.
    generator_.generate_header();

    // generate SVG root element opening tag.
    // the root element defines the size of the image,
    // which is taken from the map's dimensions.
    svg::root_output_attributes root_attributes(width_, height_);
    generator_.generate_opening_root(root_attributes);

    boost::optional<color> const& bgcolor = map.background();
    if(bgcolor)
    {
        // generate background color as a rectangle that spans the whole image.
        svg::rect_output_attributes bg_attributes(0, 0, width_, height_, *bgcolor);
        generator_.generate_rect(bg_attributes);
    }
}

template <typename T>
void svg_renderer<T>::end_map_processing(Map const& map)
{
    // generate SVG root element closing tag.
    generator_.generate_closing_root();

    MAPNIK_LOG_DEBUG(svg_renderer) << "svg_renderer: End map processing";
}

template <typename T>
void svg_renderer<T>::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(svg_renderer) << "svg_renderer: Start layer processing=" << lay.name();
}

template <typename T>
void svg_renderer<T>::end_layer_processing(layer const& lay)
{
    MAPNIK_LOG_DEBUG(svg_renderer) << "svg_renderer: End layer processing=" << lay.name();
}

template class svg_renderer<std::ostream_iterator<char> >;
}
