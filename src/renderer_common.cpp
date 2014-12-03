/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/attribute.hpp>

namespace mapnik {

renderer_common::renderer_common(Map const& map, unsigned width, unsigned height, double scale_factor,
                                 attributes const& vars,
                                 view_transform && t,
                                 std::shared_ptr<label_collision_detector4> detector)
   : width_(width),
     height_(height),
     scale_factor_(scale_factor),
     vars_(vars),
     shared_font_library_(std::make_shared<font_library>()),
     font_library_(*shared_font_library_),
     font_manager_(font_library_,map.get_font_file_mapping(),map.get_font_memory_cache()),
     query_extent_(),
     t_(t),
     detector_(detector)
{}

renderer_common::renderer_common(Map const &m, attributes const& vars, unsigned offset_x, unsigned offset_y,
                                 unsigned width, unsigned height, double scale_factor)
   : renderer_common(m, width, height, scale_factor,
                     vars,
                     view_transform(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
                     std::make_shared<label_collision_detector4>(
                        box2d<double>(-m.max_buffer_size(), -m.max_buffer_size(),
                                      m.width() + m.max_buffer_size(), m.height() + m.max_buffer_size())))
{}

renderer_common::renderer_common(Map const &m, attributes const& vars, unsigned offset_x, unsigned offset_y,
                                 unsigned width, unsigned height, double scale_factor,
                                 std::shared_ptr<label_collision_detector4> detector)
   : renderer_common(m, width, height, scale_factor,
                     vars,
                     view_transform(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
                     detector)
{}

renderer_common::renderer_common(Map const &m, request const &req, attributes const& vars, unsigned offset_x, unsigned offset_y,
                                 unsigned width, unsigned height, double scale_factor)
   : renderer_common(m, width, height, scale_factor,
                     vars,
                     view_transform(req.width(),req.height(),req.extent(),offset_x,offset_y),
                     std::make_shared<label_collision_detector4>(
                        box2d<double>(-req.buffer_size(), -req.buffer_size(),
                                      req.width() + req.buffer_size() ,req.height() + req.buffer_size())))
{}

}
