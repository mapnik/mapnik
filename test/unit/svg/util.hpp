/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef TEST_UNIT_SVG_UTIL_HPP
#define TEST_UNIT_SVG_UTIL_HPP

#include <cmath>

namespace {

template<int N = 6>
struct vertex_equal
{
    template<typename T>
    bool operator()(T const& lhs, T const& rhs) const
    {
        static const double eps = 1.0 / std::pow(10, N);
        return (std::fabs(std::get<0>(lhs) - std::get<0>(rhs)) < eps) &&
               (std::fabs(std::get<1>(lhs) - std::get<1>(rhs)) < eps) && std::get<2>(lhs) == std::get<2>(rhs);
    }
};
} // namespace

#endif // TEST_UNIT_SVG_UTIL_HPP
