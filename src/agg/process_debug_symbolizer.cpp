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
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/label_collision_detector.hpp>

namespace mapnik {

void draw_rect(image_32 &pixmap, box2d<double> const& box)
{
    int x0 = static_cast<int>(box.minx());
    int x1 = static_cast<int>(box.maxx());
    int y0 = static_cast<int>(box.miny());
    int y1 = static_cast<int>(box.maxy());
    unsigned color1 = 0xff0000ff;
    for (int x=x0; x<x1; x++)
    {
        pixmap.setPixel(x, y0, color1);
        pixmap.setPixel(x, y1, color1);
    }
    for (int y=y0; y<y1; y++)
    {
        pixmap.setPixel(x0, y, color1);
        pixmap.setPixel(x1, y, color1);
    }
}

template <typename T>
void agg_renderer<T>::process(debug_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)
{
    label_collision_detector4::query_iterator itr = detector_->begin(), end = detector_->end();
    for (;itr!=end; itr++)
    {
        draw_rect(pixmap_, itr->box);
    }
}

template void agg_renderer<image_32>::process(debug_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}

