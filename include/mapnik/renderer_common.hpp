/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/view_transform.hpp>    // for view_transform
#include <mapnik/attribute.hpp>
#include <mapnik/util/noncopyable.hpp>

// fwd declarations to speed up compile
namespace mapnik {
  class label_collision_detector4;
  class Map;
  class request;
//  class attributes;
}

namespace mapnik {

struct renderer_common : private util::noncopyable
{
    using detector_ptr = std::shared_ptr<label_collision_detector4>;

    renderer_common(Map const &m, attributes const& vars, unsigned offset_x, unsigned offset_y,
                       unsigned width, unsigned height, double scale_factor);
    renderer_common(Map const &m, attributes const& vars, unsigned offset_x, unsigned offset_y,
                       unsigned width, unsigned height, double scale_factor,
                       detector_ptr detector);
    renderer_common(Map const &m, request const &req, attributes const& vars, unsigned offset_x, unsigned offset_y,
                       unsigned width, unsigned height, double scale_factor);
    ~renderer_common();

    unsigned width_;
    unsigned height_;
    double scale_factor_;
    attributes vars_;
    // TODO: dirty hack for cairo renderer, figure out how to remove this
    std::shared_ptr<font_library> shared_font_library_;
    font_library & font_library_;
    face_manager_freetype font_manager_;
    box2d<double> query_extent_;
    view_transform t_;
    detector_ptr detector_;

protected:
    // it's desirable to keep this class implicitly noncopyable to prevent
    // inadvertent copying from other places;
    // this copy constructor is therefore protected and should only be used
    // by virtual_renderer_common
    renderer_common(renderer_common const& other);

private:
    renderer_common(Map const &m, unsigned width, unsigned height, double scale_factor,
                    attributes const& vars, view_transform && t, detector_ptr detector);
};

}

#endif /* MAPNIK_RENDERER_COMMON_HPP */
