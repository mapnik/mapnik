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

#include <mapnik/geometry_envelope.hpp>
#include <mapnik/geometry_envelope_impl.hpp>
#include <mapnik/text/symbolizer_helpers.hpp>
namespace mapnik {
namespace geometry {

template MAPNIK_DECL mapnik::box2d<double> envelope(geometry<double> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(mapnik::base_symbolizer_helper::geometry_cref const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(geometry_empty const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(point<double> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(line_string<double> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(linear_ring<double> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(polygon<double> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(multi_point<double> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(multi_line_string<double> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(multi_polygon<double> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(geometry_collection<double> const& geom);

template MAPNIK_DECL mapnik::box2d<double> envelope(geometry<std::int64_t> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(point<std::int64_t> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(line_string<std::int64_t> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(linear_ring<std::int64_t> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(polygon<std::int64_t> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(multi_point<std::int64_t> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(multi_line_string<std::int64_t> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(multi_polygon<std::int64_t> const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(geometry_collection<std::int64_t> const& geom);

} // end ns geometry
} // end ns mapnik
