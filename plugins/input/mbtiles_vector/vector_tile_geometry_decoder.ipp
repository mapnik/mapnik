// SPDX-License-Identifier: BSD-3-Clause
/**
 * Copyright (c) MapBox
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * - Neither the name "MapBox" nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//protozero
#include <protozero/pbf_reader.hpp>

//mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/geometry.hpp>
#if defined(DEBUG)
#include <mapnik/debug.hpp>
#endif

//std
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace mapnik
{

namespace vector_tile_impl
{

namespace detail
{

template <typename value_type>
inline double calculate_segment_area(value_type const x0, value_type const y0, value_type const x1, value_type const y1)
{
    return (static_cast<double>(x0) * static_cast<double>(y1)) - (static_cast<double>(y0) * static_cast<double>(x1));
}

inline bool area_is_clockwise(double area)
{
    return (area < 0.0);
}

template <typename value_type>
inline bool scaling_reversed_orientation(value_type const scale_x_, value_type const scale_y_)
{
    return (scale_x_ * scale_y_) < 0;
}

template <typename value_type>
inline void move_cursor(value_type & x, value_type & y, std::int32_t dx, std::int32_t dy)
{
    x += static_cast<value_type>(dx);
    y += static_cast<value_type>(dy);
}


template <typename value_type>
inline value_type get_point_value(value_type const val,
                                  double const scale_val,
                                  double const tile_loc)
{
    return (tile_loc + static_cast<value_type>(std::round(static_cast<double>(val) / scale_val)));
}

template <>
inline double get_point_value<double>(double const val,
                                      double const scale_val,
                                      double const tile_loc)
{
    return tile_loc + (val / scale_val);
}

constexpr std::size_t max_reserve()
{
    // Based on int64_t geometry being 16 bytes in size and 
    // maximum allocation size of 1 MB.
    return (1024 * 1024) / 16;
}

template <typename geom_value_type>
void decode_point(mapnik::geometry::geometry<geom_value_type> & geom,
                  GeometryPBF & paths,
                  geom_value_type const tile_x,
                  geom_value_type const tile_y,
                  double const scale_x,
                  double const scale_y,
                  mapnik::box2d<double> const& bbox)
{
    typename GeometryPBF::command cmd;
    using pbf_value_type = GeometryPBF::value_type;
    pbf_value_type x1, y1;
    mapnik::geometry::multi_point<geom_value_type> mp;
    #if defined(DEBUG)
    std::uint32_t previous_len = 0;
    #endif
    // Find first moveto inside bbox and then reserve points from size of geometry.
    while (true)
    {
        cmd = paths.point_next(x1, y1);
        geom_value_type x1_ = get_point_value<geom_value_type>(x1, scale_x, tile_x);
        geom_value_type y1_ = get_point_value<geom_value_type>(y1, scale_y, tile_y);
        if (cmd == GeometryPBF::end)
        {
            geom = mapnik::geometry::geometry_empty();
            return;
        }
        else if (bbox.intersects(x1_, y1_))
        {
            #if defined(DEBUG)
            if (previous_len <= paths.get_length() && !paths.already_had_error)
            {
                MAPNIK_LOG_WARN(decode_point) << "warning: encountered POINT geometry that might have MOVETO commands repeated that could be fewer commands";
                paths.already_had_error = true;
            }
            previous_len = paths.get_length();
            #endif
            constexpr std::size_t max_size = max_reserve();
            if (paths.get_length() + 1 < max_size)
            {
                mp.reserve(paths.get_length() + 1);
            }
            else
            {
                mp.reserve(max_size);
            }
            mp.emplace_back(x1_, y1_);
            break;
        }
    }
    while ((cmd = paths.point_next(x1, y1)) != GeometryPBF::end)
    {
        #if defined(DEBUG)
        if (previous_len <= paths.get_length() && !paths.already_had_error)
        {
            MAPNIK_LOG_WARN(decode_point) << "warning: encountered POINT geometry that might have MOVETO commands repeated that could be fewer commands";
            paths.already_had_error = true;
        }
        previous_len = paths.get_length();
        #endif
        // TODO: consider profiling and trying to optimize this further
        // when all points are within the bbox filter then the `mp.reserve` should be
        // perfect, but when some points are thrown out we will allocate more than needed
        // the "all points intersect" case I think is going to be more common/important
        // however worth a future look to see if the "some or few points intersect" can be optimized
        geom_value_type x1_ = get_point_value<geom_value_type>(x1, scale_x, tile_x);
        geom_value_type y1_ = get_point_value<geom_value_type>(y1, scale_y, tile_y);
        if (!bbox.intersects(x1_, y1_))
        {
            continue;
        }
        mp.emplace_back(x1_, y1_);
    }
    std::size_t num_points = mp.size();
    if (num_points == 0)
    {
        geom = mapnik::geometry::geometry_empty();
    }
    else if (num_points == 1)
    {
        geom = std::move(mp[0]);
    }
    else if (num_points > 1)
    {
        // return multipoint
        geom = std::move(mp);
    }
}

template <typename geom_value_type>
void decode_linestring(mapnik::geometry::geometry<geom_value_type> & geom,
                       GeometryPBF & paths,
                       geom_value_type const tile_x,
                       geom_value_type const tile_y,
                       double scale_x,
                       double scale_y,
                       mapnik::box2d<double> const& bbox,
                       unsigned version)
{
    using pbf_value_type = GeometryPBF::value_type;
    typename GeometryPBF::command cmd;
    pbf_value_type x0, y0;
    pbf_value_type x1, y1;
    geom_value_type x0_, y0_;
    geom_value_type x1_, y1_;
    mapnik::geometry::multi_line_string<geom_value_type> multi_line;
    #if defined(DEBUG)
    std::uint32_t previous_len = 0;
    #endif
    mapnik::box2d<double> part_env;
    cmd = paths.line_next(x0, y0, false);
    if (cmd == GeometryPBF::end)
    {
        geom = mapnik::geometry::geometry_empty();
        return;
    }
    else if (cmd != GeometryPBF::move_to)
    {
        throw std::runtime_error("Vector Tile has LINESTRING type geometry where the first command is not MOVETO.");
    }

    while (true)
    {
        cmd = paths.line_next(x1, y1, true);
        if (cmd != GeometryPBF::line_to)
        {
            if (cmd == GeometryPBF::move_to)
            {
                if (version == 1)
                {
                    // Version 1 of the spec wasn't clearly defined and therefore
                    // we shouldn't be strict on the reading of a tile that has two
                    // moveto commands that are repeated, lets ignore the previous moveto.
                    x0 = x1;
                    y0 = y1;
                    continue;
                }
                else
                {
                    throw std::runtime_error("Vector Tile has LINESTRING type geometry with repeated MOVETO commands.");
                }
            }
            else //cmd == GeometryPBF::end
            {
                if (version == 1)
                {
                    // Version 1 of the spec wasn't clearly defined and therefore
                    // we shouldn't be strict on the reading of a tile that has only a moveto
                    // command. So lets just ignore this moveto command.
                    break;
                }
                else
                {
                    throw std::runtime_error("Vector Tile has LINESTRING type geometry with a MOVETO command with no LINETO following.");
                }
            }
        }
        // add fresh line
        multi_line.emplace_back();
        auto & line = multi_line.back();
        // reserve prior
        constexpr std::size_t max_size = max_reserve();
        if (paths.get_length() + 2 < max_size)
        {
            line.reserve(paths.get_length() + 2);
        }
        else
        {
            line.reserve(max_size);
        }
        // add moveto command position
        x0_ = get_point_value<geom_value_type>(x0, scale_x, tile_x);
        y0_ = get_point_value<geom_value_type>(y0, scale_y, tile_y);
        line.emplace_back(x0_, y0_);
        part_env.init(x0_, y0_, x0_, y0_);
        // add first lineto
        x1_ = get_point_value<geom_value_type>(x1, scale_x, tile_x);
        y1_ = get_point_value<geom_value_type>(y1, scale_y, tile_y);
        line.emplace_back(x1_, y1_);
        part_env.expand_to_include(x1_, y1_);
        #if defined(DEBUG)
        previous_len = paths.get_length();
        #endif
        while ((cmd = paths.line_next(x1, y1, true)) == GeometryPBF::line_to)
        {
            x1_ = get_point_value<geom_value_type>(x1, scale_x, tile_x);
            y1_ = get_point_value<geom_value_type>(y1, scale_y, tile_y);
            line.emplace_back(x1_, y1_);
            part_env.expand_to_include(x1_, y1_);
            #if defined(DEBUG)
            if (previous_len <= paths.get_length() && !paths.already_had_error)
            {
                MAPNIK_LOG_WARN(decode_linestring) << "warning: encountered LINESTRING geometry that might have LINETO commands repeated that could be fewer commands";
                paths.already_had_error = true;
            }
            previous_len = paths.get_length();
            #endif
        }
        if (!bbox.intersects(part_env))
        {
            // remove last linestring
            multi_line.pop_back();
        }
        if (cmd == GeometryPBF::end)
        {
            break;
        }
        // else we are guaranteed it is a moveto
        x0 = x1;
        y0 = y1;
    }

    std::size_t num_lines = multi_line.size();
    if (num_lines == 0)
    {
        geom = mapnik::geometry::geometry_empty();
    }
    else if (num_lines == 1)
    {
        auto itr = std::make_move_iterator(multi_line.begin());
        if (itr->size() > 1)
        {
            geom = std::move(*itr);
        }
        else
        {
            geom = mapnik::geometry::geometry_empty();
        }
    }
    else if (num_lines > 1)
    {
        geom = std::move(multi_line);
    }
}

template <typename geom_value_type>
void decode_polygon(mapnik::geometry::geometry<geom_value_type> & geom,
                    GeometryPBF & paths,
                    geom_value_type const tile_x,
                    geom_value_type const tile_y,
                    double scale_x,
                    double scale_y,
                    mapnik::box2d<double> const& bbox,
                    unsigned version)
{
    typename GeometryPBF::command cmd;
    using pbf_value_type = GeometryPBF::value_type;
    pbf_value_type x0, y0;
    pbf_value_type x1, y1;
    pbf_value_type x2, y2;
    geom_value_type x0_, y0_;
    geom_value_type x1_, y1_;
    geom_value_type x2_, y2_;
    #if defined(DEBUG)
    std::uint32_t previous_len;
    #endif
    double ring_area = 0.0;
    bool first_ring = true;
    bool first_ring_is_clockwise = false;
    bool last_exterior_not_included = false;
    std::vector<mapnik::geometry::linear_ring<geom_value_type> > rings;
    std::vector<bool> rings_exterior;
    mapnik::box2d<double> part_env;
    cmd = paths.ring_next(x0, y0, false);
    if (cmd == GeometryPBF::end)
    {
        geom = mapnik::geometry::geometry_empty();
        return;
    }
    else if (cmd != GeometryPBF::move_to)
    {
        throw std::runtime_error("Vector Tile has POLYGON type geometry where the first command is not MOVETO.");
    }

    while (true)
    {
        cmd = paths.ring_next(x1, y1, true);
        if (cmd != GeometryPBF::line_to)
        {
            if (cmd == GeometryPBF::close && version == 1)
            {
                // Version 1 of the specification was not clear on the command requirements for polygons
                // lets just to recover from this situation.
                cmd = paths.ring_next(x0, y0, false);
                if (cmd == GeometryPBF::end)
                {
                    break;
                }
                else if (cmd == GeometryPBF::move_to)
                {
                    continue;
                }
                else if (cmd == GeometryPBF::close)
                {
                    throw std::runtime_error("Vector Tile has POLYGON type geometry where a CLOSE is followed by a CLOSE.");
                }
                else // cmd == GeometryPBF::line_to
                {
                    throw std::runtime_error("Vector Tile has POLYGON type geometry where a CLOSE is followed by a LINETO.");
                }
            }
            else // cmd == end || cmd == move_to
            {
                throw std::runtime_error("Vector Tile has POLYGON type geometry with a MOVETO command with out at least two LINETOs and CLOSE following.");
            }
        }
        #if defined(DEBUG)
        previous_len = paths.get_length();
        #endif
        cmd = paths.ring_next(x2, y2, true);
        if (cmd != GeometryPBF::line_to)
        {
            if (cmd == GeometryPBF::close && version == 1)
            {
                // Version 1 of the specification was not clear on the command requirements for polygons
                // lets just to recover from this situation.
                cmd = paths.ring_next(x0, y0, false);
                if (cmd == GeometryPBF::end)
                {
                    break;
                }
                else if (cmd == GeometryPBF::move_to)
                {
                    continue;
                }
                else if (cmd == GeometryPBF::close)
                {
                    throw std::runtime_error("Vector Tile has POLYGON type geometry where a CLOSE is followed by a CLOSE.");
                }
                else // cmd == GeometryPBF::line_to
                {
                    throw std::runtime_error("Vector Tile has POLYGON type geometry where a CLOSE is followed by a LINETO.");
                }
            }
            else // cmd == end || cmd == move_to
            {
                throw std::runtime_error("Vector Tile has POLYGON type geometry with a MOVETO command with out at least two LINETOs and CLOSE following.");
            }
        }
        // add new ring to start adding to
        rings.emplace_back();
        auto & ring = rings.back();
        // reserve prior
        constexpr std::size_t max_size = max_reserve();
        if (paths.get_length() + 4 < max_size)
        {
            ring.reserve(paths.get_length() + 4);
        }
        else
        {
            ring.reserve(max_size);
        }
        // add moveto command position
        x0_ = get_point_value<geom_value_type>(x0, scale_x, tile_x);
        y0_ = get_point_value<geom_value_type>(y0, scale_y, tile_y);
        ring.emplace_back(x0_, y0_);
        part_env.init(x0_, y0_, x0_, y0_);
        // add first lineto
        x1_ = get_point_value<geom_value_type>(x1, scale_x, tile_x);
        y1_ = get_point_value<geom_value_type>(y1, scale_y, tile_y);
        ring.emplace_back(x1_, y1_);
        part_env.expand_to_include(x1_, y1_);
        ring_area += calculate_segment_area(x0, y0, x1, y1);
        // add second lineto
        x2_ = get_point_value<geom_value_type>(x2, scale_x, tile_x);
        y2_ = get_point_value<geom_value_type>(y2, scale_y, tile_y);
        ring.emplace_back(x2_, y2_);
        part_env.expand_to_include(x2_, y2_);
        ring_area += calculate_segment_area(x1, y1, x2, y2);
        x1 = x2;
        y1 = y2;
        #if defined(DEBUG)
        if (previous_len <= paths.get_length() && !paths.already_had_error)
        {
            MAPNIK_LOG_WARN(read_rings) << "warning: encountered POLYGON geometry that might have LINETO commands repeated that could be fewer commands";
            paths.already_had_error = true;
        }
        previous_len = paths.get_length();
        #endif
        while ((cmd = paths.ring_next(x2, y2, true)) == GeometryPBF::line_to)
        {
            x2_ = get_point_value<geom_value_type>(x2, scale_x, tile_x);
            y2_ = get_point_value<geom_value_type>(y2, scale_y, tile_y);
            ring.emplace_back(x2_, y2_);
            part_env.expand_to_include(x2_, y2_);
            ring_area += calculate_segment_area(x1, y1, x2, y2);
            x1 = x2;
            y1 = y2;
            #if defined(DEBUG)
            if (previous_len <= paths.get_length() && !paths.already_had_error)
            {
                MAPNIK_LOG_WARN(read_rings) << "warning: encountered POLYGON geometry that might have LINETO commands repeated that could be fewer commands";
                paths.already_had_error = true;
            }
            previous_len = paths.get_length();
            #endif
        }
        // Make sure we are now on a close command
        if (cmd != GeometryPBF::close)
        {
            throw std::runtime_error("Vector Tile has POLYGON type geometry with a ring not closed by a CLOSE command.");
        }
        if (ring.back().x != x0_ || ring.back().y != y0_)
        {
            // If the previous lineto didn't already close the polygon (WHICH IT SHOULD NOT)
            // close out the polygon ring.
            ring.emplace_back(x0_, y0_);
            ring_area += calculate_segment_area(x1, y1, x0, y0);
        }
        if (ring.size() > 3)
        {
            if (first_ring)
            {
                first_ring_is_clockwise = area_is_clockwise(ring_area);
                if (version != 1 && first_ring_is_clockwise)
                {
                    throw std::runtime_error("Vector Tile has POLYGON with first ring clockwise. It is not valid according to v2 of VT spec.");
                }
                first_ring = false;
            }
            bool is_exterior = (first_ring_is_clockwise == area_is_clockwise(ring_area));
            if ((!is_exterior && last_exterior_not_included) || !bbox.intersects(part_env))
            {
                // remove last linestring
                if (is_exterior)
                {
                    last_exterior_not_included = true;
                }
                rings.pop_back();
            }
            else
            {
                if (is_exterior)
                {
                    last_exterior_not_included = false;
                }
                rings_exterior.push_back(is_exterior);
            }
        }
        else
        {
            rings.pop_back();
        }
        ring_area = 0.0;

        cmd = paths.ring_next(x0, y0, false);
        if (cmd == GeometryPBF::end)
        {
            break;
        }
        else if (cmd != GeometryPBF::move_to)
        {
            if (cmd == GeometryPBF::close)
            {
                throw std::runtime_error("Vector Tile has POLYGON type geometry where a CLOSE is followed by a CLOSE.");
            }
            else // cmd == GeometryPBF::line_to
            {
                throw std::runtime_error("Vector Tile has POLYGON type geometry where a CLOSE is followed by a LINETO.");
            }
        }
    }

    if (rings.size() == 0)
    {
        geom = mapnik::geometry::geometry_empty();
        return;
    }

    bool reverse_rings = (scaling_reversed_orientation(scale_x, scale_y) != first_ring_is_clockwise);
    auto rings_itr = std::make_move_iterator(rings.begin());
    auto rings_end = std::make_move_iterator(rings.end());
    mapnik::geometry::multi_polygon<geom_value_type> multi_poly;
    for (std::size_t i = 0; rings_itr != rings_end; ++rings_itr,++i)
    {
        if (rings_exterior[i]) multi_poly.emplace_back();
        auto & poly = multi_poly.back();
        if (reverse_rings)
        {
            std::reverse(rings_itr->begin(), rings_itr->end());
        }
        poly.push_back(std::move(*rings_itr));
    }
    auto num_poly = multi_poly.size();
    if (num_poly == 1)
    {
        auto itr = std::make_move_iterator(multi_poly.begin());
        geom = std::move(*itr);
    }
    else
    {
        geom = std::move(multi_poly);
    }
}

} // end ns detail

GeometryPBF::GeometryPBF(pbf_itr const& geo_iterator)
    : geo_itr_(geo_iterator.begin()),
      geo_end_itr_(geo_iterator.end()),
      x(0),
      y(0),
      ox(0),
      oy(0),
      length(0),
      cmd(move_to)
{
    #if defined(DEBUG)
    already_had_error = false;
    #endif
}

typename GeometryPBF::command GeometryPBF::point_next(value_type & rx, value_type & ry)
{
    if (length == 0)
    {
        if (geo_itr_ != geo_end_itr_)
        {
            uint32_t cmd_length = static_cast<uint32_t>(*geo_itr_++);
            cmd = cmd_length & 0x7;
            length = cmd_length >> 3;
            if (cmd == move_to)
            {
                if (length == 0)
                {
                    throw std::runtime_error("Vector Tile has POINT geometry with a MOVETO command that has a command count of zero");
                }
            }
            else
            {
                if (cmd == line_to)
                {
                    throw std::runtime_error("Vector Tile has POINT type geometry with a LINETO command.");
                }
                else if (cmd == close)
                {
                    throw std::runtime_error("Vector Tile has POINT type geometry with a CLOSE command.");
                }
                else
                {
                    throw std::runtime_error("Vector Tile has POINT type geometry with an unknown command.");
                }
            }
        }
        else
        {
            return end;
        }
    }

    --length;
    // It is possible for the next to lines to throw because we can not check the length
    // of the buffer to ensure that it is long enough.
    // If an exception occurs it will likely be a end_of_buffer_exception with the text:
    // "end of buffer exception"
    // While this error message is not verbose a try catch here would slow down processing.
    int32_t dx = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
    int32_t dy = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
    detail::move_cursor(x, y, dx, dy);
    rx = x;
    ry = y;
    return move_to;
}

typename GeometryPBF::command GeometryPBF::line_next(value_type & rx,
                                                     value_type & ry,
                                                     bool skip_lineto_zero)
{
    if (length == 0)
    {
        if (geo_itr_ != geo_end_itr_)
        {
            uint32_t cmd_length = static_cast<uint32_t>(*geo_itr_++);
            cmd = cmd_length & 0x7;
            length = cmd_length >> 3;
            if (cmd == move_to)
            {
                if (length != 1)
                {
                    throw std::runtime_error("Vector Tile has LINESTRING with a MOVETO command that is given more then one pair of parameters or not enough parameters are provided");
                }
                --length;
                // It is possible for the next to lines to throw because we can not check the length
                // of the buffer to ensure that it is long enough.
                // If an exception occurs it will likely be a end_of_buffer_exception with the text:
                // "end of buffer exception"
                // While this error message is not verbose a try catch here would slow down processing.
                int32_t dx = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
                int32_t dy = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
                detail::move_cursor(x, y, dx, dy);
                rx = x;
                ry = y;
                return move_to;
            }
            else if (cmd == line_to)
            {
                if (length == 0)
                {
                    throw std::runtime_error("Vector Tile has geometry with LINETO command that is not followed by a proper number of parameters");
                }
            }
            else
            {
                if (cmd == close)
                {
                    throw std::runtime_error("Vector Tile has LINESTRING type geometry with a CLOSE command.");
                }
                else
                {
                    throw std::runtime_error("Vector Tile has LINESTRING type geometry with an unknown command.");
                }
            }
        }
        else
        {
            return end;
        }
    }

    --length;
    // It is possible for the next to lines to throw because we can not check the length
    // of the buffer to ensure that it is long enough.
    // If an exception occurs it will likely be a end_of_buffer_exception with the text:
    // "end of buffer exception"
    // While this error message is not verbose a try catch here would slow down processing.
    int32_t dx = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
    int32_t dy = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
    if (skip_lineto_zero && dx == 0 && dy == 0)
    {
        // We are going to skip this vertex as the point doesn't move call line_next again
        return line_next(rx, ry, true);
    }
    detail::move_cursor(x, y, dx, dy);
    rx = x;
    ry = y;
    return line_to;
}

typename GeometryPBF::command GeometryPBF::ring_next(value_type & rx,
                                                     value_type & ry,
                                                     bool skip_lineto_zero)
{
    if (length == 0)
    {
        if (geo_itr_ != geo_end_itr_)
        {
            uint32_t cmd_length = static_cast<uint32_t>(*geo_itr_++);
            cmd = cmd_length & 0x7;
            length = cmd_length >> 3;
            if (cmd == move_to)
            {
                if (length != 1)
                {
                    throw std::runtime_error("Vector Tile has POLYGON with a MOVETO command that is given more then one pair of parameters or not enough parameters are provided");
                }
                --length;
                // It is possible for the next two lines to throw because we can not check the length
                // of the buffer to ensure that it is long enough.
                // If an exception occurs it will likely be a end_of_buffer_exception with the text:
                // "end of buffer exception"
                // While this error message is not verbose a try catch here would slow down processing.
                int32_t dx = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
                int32_t dy = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
                detail::move_cursor(x, y, dx, dy);
                rx = x;
                ry = y;
                ox = x;
                oy = y;
                return move_to;
            }
            else if (cmd == line_to)
            {
                if (length == 0)
                {
                    throw std::runtime_error("Vector Tile has geometry with LINETO command that is not followed by a proper number of parameters");
                }
            }
            else if (cmd == close)
            {
                // Just set length in case a close command provides an invalid number here.
                // While we could throw because V2 of the spec declares it incorrect, this is not
                // difficult to fix and has no effect on the results.
                length = 0;
                rx = ox;
                ry = oy;
                return close;
            }
            else
            {
                throw std::runtime_error("Vector Tile has POLYGON type geometry with an unknown command.");
            }
        }
        else
        {
            return end;
        }
    }

    --length;
    // It is possible for the next to lines to throw because we can not check the length
    // of the buffer to ensure that it is long enough.
    // If an exception occurs it will likely be a end_of_buffer_exception with the text:
    // "end of buffer exception"
    // While this error message is not verbose a try catch here would slow down processing.
    int32_t dx = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
    int32_t dy = protozero::decode_zigzag32(static_cast<uint32_t>(*geo_itr_++));
    if (skip_lineto_zero && dx == 0 && dy == 0)
    {
        // We are going to skip this vertex as the point doesn't move call ring_next again
        return ring_next(rx, ry, true);
    }
    detail::move_cursor(x, y, dx, dy);
    rx = x;
    ry = y;
    return line_to;
}

template <typename value_type>
mapnik::geometry::geometry<value_type> decode_geometry(GeometryPBF & paths,
        mvt_message::geom_type geom_type,
        unsigned version,
        value_type tile_x,
        value_type tile_y,
        double scale_x,
        double scale_y,
        mapnik::box2d<double> const& bbox)
{
    mapnik::geometry::geometry<value_type> geom; // output geometry
    switch (geom_type)
    {
        case mvt_message::geom_type::point:
        {
            detail::decode_point<value_type>(geom, paths, tile_x, tile_y, scale_x, scale_y, bbox);
            break;
        }
        case mvt_message::geom_type::linestring:
        {
            detail::decode_linestring<value_type>(geom, paths, tile_x, tile_y, scale_x, scale_y, bbox, version);
            break;
        }
        case mvt_message::geom_type::polygon:
        {
            detail::decode_polygon<value_type>(geom, paths, tile_x, tile_y, scale_x, scale_y, bbox, version);
            break;
        }
        case mvt_message::geom_type::unknown:
        default:
        {
            // This was changed to not throw as unknown according to v2 of spec can simply be ignored and doesn't require
            // it failing the processing
            geom = mapnik::geometry::geometry_empty();
            break;
        }
    }
    return geom;
}

template <typename value_type>
mapnik::geometry::geometry<value_type> decode_geometry(GeometryPBF & paths,
        mvt_message::geom_type geom_type,
        unsigned version,
        value_type tile_x,
        value_type tile_y,
        double scale_x,
        double scale_y)
{
    mapnik::box2d<double> bbox(std::numeric_limits<double>::lowest(),
                               std::numeric_limits<double>::lowest(),
                               std::numeric_limits<double>::max(),
                               std::numeric_limits<double>::max());
    return decode_geometry<value_type>(paths, geom_type, version, tile_x, tile_y, scale_x, scale_y, bbox);
}

} // end ns vector_tile_impl

} // end ns mapnik
