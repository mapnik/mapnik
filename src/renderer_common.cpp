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

#include <mapnik/renderer_common.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/map.hpp>
#include <mapnik/request.hpp>

namespace mapnik {

renderer_common::renderer_common(unsigned width, unsigned height, double scale_factor,
                                 CoordTransform &&t, std::shared_ptr<label_collision_detector4> detector)
   : width_(width),
     height_(height),
     scale_factor_(scale_factor),
     shared_font_engine_(std::make_shared<freetype_engine>()),
     font_engine_(*shared_font_engine_),
     font_manager_(font_engine_),
     query_extent_(),
     t_(t),
     detector_(detector)
{}

renderer_common::renderer_common(Map const &m, unsigned offset_x, unsigned offset_y,
                                 unsigned width, unsigned height, double scale_factor)
   : renderer_common(width, height, scale_factor,
                     CoordTransform(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
                     std::make_shared<label_collision_detector4>(
                        box2d<double>(-m.buffer_size(), -m.buffer_size(), 
                                      m.width() + m.buffer_size() ,m.height() + m.buffer_size())))
{}

renderer_common::renderer_common(Map const &m, unsigned offset_x, unsigned offset_y,
                                 unsigned width, unsigned height, double scale_factor,
                                 std::shared_ptr<label_collision_detector4> detector)
   : renderer_common(width, height, scale_factor,
                     CoordTransform(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
                     detector)
{}

renderer_common::renderer_common(request const &req, unsigned offset_x, unsigned offset_y,
                                 unsigned width, unsigned height, double scale_factor)
   : renderer_common(width, height, scale_factor,
                     CoordTransform(req.width(),req.height(),req.extent(),offset_x,offset_y),
                     std::make_shared<label_collision_detector4>(
                        box2d<double>(-req.buffer_size(), -req.buffer_size(), 
                                      req.width() + req.buffer_size() ,req.height() + req.buffer_size())))
{}

renderer_common::renderer_common(renderer_common const &other)
   : width_(other.width_),
     height_(other.height_),
     scale_factor_(other.scale_factor_),
     shared_font_engine_(other.shared_font_engine_),
     font_engine_(*shared_font_engine_),
     font_manager_(font_engine_),
     query_extent_(),
     t_(other.t_),
     detector_(other.detector_)
{
}

}
