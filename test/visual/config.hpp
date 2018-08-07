/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef CONFIG_HPP
#define CONFIG_HPP

// stl
#include <vector>
#include <set>
#include <string>
#include <chrono>

// boost
#include <boost/filesystem.hpp>

#include <mapnik/geometry/box2d.hpp>

namespace visual_tests
{

struct map_size
{
    map_size(std::size_t _width, std::size_t _height) : width(_width), height(_height) { }
    map_size() { }
    std::size_t width = 0;
    std::size_t height = 0;
    std::size_t margin_left = 0;
    std::size_t margin_right = 0;
    std::size_t margin_top = 0;
    std::size_t margin_bottom = 0;

    bool has_margin() const
    {
        return 0 != (margin_left | margin_right | margin_top | margin_bottom);
    }

    void inflate_by_margin(mapnik::box2d<double> & box) const
    {
        std::size_t xm = margin_left + margin_right;
        if (xm > 0 && xm < width)
        {
            double pixel_width = box.width() / (width - xm);
            box.minx_ -= pixel_width * margin_left;
            box.maxx_ += pixel_width * margin_right;
        }
        std::size_t ym = margin_top + margin_bottom;
        if (ym > 0 && ym < height)
        {
            double pixel_height = box.height() / (height - ym);
            // FIXME this will swap top/bottom margins
            // if map coordinate system has `y` growing in
            // the same direction as screen coordinates
            box.maxy_ += pixel_height * margin_top;
            box.miny_ -= pixel_height * margin_bottom;
        }
    }
};

struct config
{
    config() : status(true),
               scales({ 1.0, 2.0 }),
               sizes({ { 500, 100 } }),
               tiles({ { 1, 1 } }),
               bbox(),
               ignored_renderers() { }

    bool status;
    std::vector<double> scales;
    std::vector<map_size> sizes;
    std::vector<map_size> tiles;
    mapnik::box2d<double> bbox;
    std::set<std::string> ignored_renderers;
};

enum result_state : std::uint8_t
{
    STATE_OK,
    STATE_FAIL,
    STATE_ERROR,
    STATE_OVERWRITE
};

struct result
{
    std::string name;
    result_state state;
    std::string renderer_name;
    map_size size;
    map_size tiles;
    double scale_factor;
    boost::filesystem::path actual_image_path;
    boost::filesystem::path reference_image_path;
    std::string error_message;
    unsigned diff;
    std::chrono::high_resolution_clock::duration duration;
};

using result_list = std::vector<result>;

}

#endif
