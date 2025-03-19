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

#ifndef VECTOR_TILE_GEOMETRY_DECODER_HPP_
#define VECTOR_TILE_GEOMETRY_DECODER_HPP_

//protozero
#include <protozero/pbf_reader.hpp>

//mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/geometry.hpp>
#if defined(DEBUG)
#include <mapnik/debug.hpp>
#endif
#include "mvt_message.hpp"

//std
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace mapnik
{

namespace vector_tile_impl
{

// NOTE: this object is for one-time use.  Once you've progressed to the end
//       by calling next(), to re-iterate, you must construct a new object
class GeometryPBF
{
public:
    using value_type = std::int64_t;
    using iterator_type = protozero::pbf_reader::const_uint32_iterator;
    using pbf_itr = protozero::iterator_range<iterator_type>;

    explicit GeometryPBF(pbf_itr const& geo_iterator);

    enum command : uint8_t
    {
        end = 0,
        move_to = 1,
        line_to = 2,
        close = 7
    };

    uint32_t get_length() const
    {
        return length;
    }

    command point_next(value_type & rx, value_type & ry);
    command line_next(value_type & rx, value_type & ry, bool skip_lineto_zero);
    command ring_next(value_type & rx, value_type & ry, bool skip_lineto_zero);

private:
    iterator_type geo_itr_;
    iterator_type geo_end_itr_;
    value_type x, y;
    value_type ox, oy;
    uint32_t length;
    uint8_t cmd;
    #if defined(DEBUG)
public:
    bool already_had_error;
    #endif
};

template <typename value_type>
mapnik::geometry::geometry<value_type> decode_geometry(GeometryPBF & paths,
        mvt_message::geom_type geom_type,
        unsigned version,
        value_type tile_x,
        value_type tile_y,
        double scale_x,
        double scale_y,
        mapnik::box2d<double> const& bbox);

template <typename value_type>
mapnik::geometry::geometry<value_type> decode_geometry(GeometryPBF & paths,
        mvt_message::geom_type geom_type,
        unsigned version,
        value_type tile_x,
        value_type tile_y,
        double scale_x,
        double scale_y);

} // end ns vector_tile_impl

} // end ns mapnik

#endif /* VECTOR_TILE_GEOMETRY_DECODER_HPP_ */
