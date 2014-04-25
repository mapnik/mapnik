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

#ifndef MAPNIK_RENDERER_COMMON_HPP
#define MAPNIK_RENDERER_COMMON_HPP

#include <mapnik/config.hpp>            // for MAPNIK_DECL
#include <mapnik/font_engine_freetype.hpp>  // for face_manager, etc
#include <mapnik/box2d.hpp>     // for box2d
#include <mapnik/ctrans.hpp>    // for CoordTransform

// fwd declarations to speed up compile
namespace mapnik {
  class label_collision_detector4;
  class Map;
  class request;
}

namespace mapnik {

struct renderer_common
{
    renderer_common(Map const &m, unsigned offset_x, unsigned offset_y, 
                       unsigned width, unsigned height, double scale_factor);
    renderer_common(Map const &m, unsigned offset_x, unsigned offset_y, 
                       unsigned width, unsigned height, double scale_factor,
                       std::shared_ptr<label_collision_detector4> detector);
    renderer_common(request const &req, unsigned offset_x, unsigned offset_y, 
                       unsigned width, unsigned height, double scale_factor);
    renderer_common(const renderer_common &);

    unsigned width_;
    unsigned height_;
    double scale_factor_;
    // TODO: dirty hack for cairo renderer, figure out how to remove this
    std::shared_ptr<freetype_engine> shared_font_engine_;
    freetype_engine &font_engine_;
    face_manager<freetype_engine> font_manager_;
    box2d<double> query_extent_;
    CoordTransform t_;
    std::shared_ptr<label_collision_detector4> detector_;

private:
    renderer_common(unsigned width, unsigned height, double scale_factor,
                       CoordTransform &&t, std::shared_ptr<label_collision_detector4> detector);
};

}

#endif /* MAPNIK_RENDERER_COMMON_HPP */

