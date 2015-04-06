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

template MAPNIK_DECL mapnik::box2d<double> envelope(geometry const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(mapnik::base_symbolizer_helper::geometry_cref const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(geometry_empty const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(point const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(line_string const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(polygon const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(multi_point const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(multi_line_string const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(multi_polygon const& geom);
template MAPNIK_DECL mapnik::box2d<double> envelope(geometry_collection const& geom);

} // end ns geometry
} // end ns mapnik
