/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_SEGMENT_HPP
#define MAPNIK_SEGMENT_HPP

#include <tuple>
#include <algorithm>

namespace mapnik
{

using segment_t = std::tuple<double,double,double,double>;

static inline bool y_order(segment_t const& first,segment_t const& second)
{
    double miny0 = std::min(std::get<1>(first), std::get<3>(first));
    double miny1 = std::min(std::get<1>(second), std::get<3>(second));
    return miny0 > miny1;
}

}

#endif // MAPNIK_SEGMENT_HPP
