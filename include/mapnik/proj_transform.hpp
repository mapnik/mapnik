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

#ifndef MAPNIK_PROJ_TRANSFORM_HPP
#define MAPNIK_PROJ_TRANSFORM_HPP

// mapnik
#include <mapnik/projection.hpp>
#include <mapnik/box2d.hpp>

// boost
#include <boost/utility.hpp>

namespace mapnik {

class MAPNIK_DECL proj_transform : private boost::noncopyable
{
public:
    proj_transform(projection const& source,
                   projection const& dest);

    bool equal() const;
    bool forward (double& x, double& y , double& z) const;
    bool backward (double& x, double& y , double& z) const;
    bool forward (double *x, double *y , double *z, int point_count) const;
    bool backward (double *x, double *y , double *z, int point_count) const;
    bool forward (box2d<double> & box) const;
    bool backward (box2d<double> & box) const;
    bool forward (box2d<double> & box, int points) const;
    bool backward (box2d<double> & box, int points) const;
    mapnik::projection const& source() const;
    mapnik::projection const& dest() const;

private:
    projection const source_;
    projection const dest_;
    bool is_source_longlat_;
    bool is_dest_longlat_;
    bool is_source_equal_dest_;
    bool wgs84_to_merc_;
};
}

#endif // MAPNIK_PROJ_TRANSFORM_HPP
