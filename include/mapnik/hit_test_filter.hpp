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

#ifndef MAPNIK_HIT_TEST_FILTER_HPP
#define MAPNIK_HIT_TEST_FILTER_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/geom_util.hpp>
// boost


namespace mapnik {
class hit_test_filter
{
public:
    hit_test_filter(double x, double y, double tol)
        : x_(x),
          y_(y),
          tol_(tol) {}

    bool pass(feature_impl & feature)
    {
        for (geometry_type & geom : feature.paths())
        {
            if (label::hit_test(geom, x_,y_,tol_))
                return true;
        }
        return false;
    }

private:
    double x_;
    double y_;
    double tol_;
};
}

#endif // MAPNIK_HIT_TEST_FILTER_HPP
