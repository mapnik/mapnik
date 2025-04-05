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

#include "vector_tile_geometry_decoder.hpp"
#include "vector_tile_geometry_decoder.ipp"

namespace mapnik
{

namespace vector_tile_impl
{

// decode geometry
template mapnik::geometry::geometry<double> decode_geometry<double>(GeometryPBF & paths, 
        mvt_message::geom_type geom_type,
        unsigned version,
        double tile_x,
        double tile_y,
        double scale_x,
        double scale_y);
template mapnik::geometry::geometry<std::int64_t> decode_geometry<std::int64_t>(GeometryPBF & paths,
        mvt_message::geom_type geom_type,
        unsigned version,
        std::int64_t tile_x,
        std::int64_t tile_y,
        double scale_x,
        double scale_y);
template mapnik::geometry::geometry<double> decode_geometry<double>(GeometryPBF & paths, 
        mvt_message::geom_type geom_type,
        unsigned version,
        double tile_x,
        double tile_y,
        double scale_x,
        double scale_y,
        mapnik::box2d<double> const& bbox);
template mapnik::geometry::geometry<std::int64_t> decode_geometry<std::int64_t>(GeometryPBF & paths,
        mvt_message::geom_type geom_type,
        unsigned version,
        std::int64_t tile_x,
        std::int64_t tile_y,
        double scale_x,
        double scale_y,
        mapnik::box2d<double> const& bbox);

} // end ns vector_tile_impl

} // end ns mapnik
