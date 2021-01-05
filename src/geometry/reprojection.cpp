/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

// mapnik
#include <mapnik/geometry/reprojection.hpp>
#include <mapnik/geometry/reprojection_impl.hpp>

namespace mapnik {

namespace geometry {

template MAPNIK_DECL geometry<double> reproject_copy(geometry<double> const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL geometry_empty reproject_copy(geometry_empty const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL point<double> reproject_copy(point<double> const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL line_string<double> reproject_copy(line_string<double> const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL polygon<double> reproject_copy(polygon<double> const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL multi_point<double> reproject_copy(multi_point<double> const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL multi_line_string<double> reproject_copy(multi_line_string<double> const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL multi_polygon<double> reproject_copy(multi_polygon<double> const& geom, proj_transform const& proj_trans, unsigned int & n_err);
template MAPNIK_DECL geometry_collection<double> reproject_copy(geometry_collection<double> const& geom, proj_transform const& proj_trans, unsigned int & n_err);

template MAPNIK_DECL geometry<double> reproject_copy(geometry<double> const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL geometry_empty reproject_copy(geometry_empty const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL point<double> reproject_copy(point<double> const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL line_string<double> reproject_copy(line_string<double> const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL polygon<double> reproject_copy(polygon<double> const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL multi_point<double> reproject_copy(multi_point<double> const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL multi_line_string<double> reproject_copy(multi_line_string<double> const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL multi_polygon<double> reproject_copy(multi_polygon<double> const& geom, projection const& source, projection const& dest, unsigned int & n_err);
template MAPNIK_DECL geometry_collection<double> reproject_copy(geometry_collection<double> const& geom, projection const& source, projection const& dest, unsigned int & n_err);

template MAPNIK_DECL bool reproject(geometry<double> & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(geometry_empty & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(point<double> & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(line_string<double> & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(polygon<double> & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(multi_point<double> & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(multi_line_string<double> & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(multi_polygon<double> & geom, proj_transform const& proj_trans);
template MAPNIK_DECL bool reproject(geometry_collection<double> & geom, proj_transform const& proj_trans);

template MAPNIK_DECL bool reproject(geometry<double> & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(geometry_empty & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(point<double> & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(line_string<double> & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(polygon<double> & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(multi_point<double> & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(multi_line_string<double> & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(multi_polygon<double> & geom, projection const& source, projection const& dest);
template MAPNIK_DECL bool reproject(geometry_collection<double> & geom, projection const& source, projection const& dest);

} // end geometry ns

} // end mapnik ns
