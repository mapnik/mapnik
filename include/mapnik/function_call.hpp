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

#ifndef MAPNIK_FUNCTION_CALL_HPP
#define MAPNIK_FUNCTION_CALL_HPP

#include <mapnik/value.hpp>
#include <functional>
#include <algorithm>
#include <cmath>

namespace mapnik {

using value_type = mapnik::value;

using unary_function_impl = std::function<value_type(value_type const&)>;
using binary_function_impl = std::function<value_type(value_type const&, value_type const&)>;

char const* unary_function_name(unary_function_impl const& fun);
char const* binary_function_name(binary_function_impl const& fun);

// functions
// exp
struct exp_impl
{
    // using type = T;
    value_type operator()(value_type const& val) const { return std::exp(val.to_double()); }
};

// log
struct log_impl
{
    // using type = T;
    value_type operator()(value_type const& val) const { return std::log(val.to_double()); }
};

// sin
struct sin_impl
{
    value_type operator()(value_type const& val) const { return std::sin(val.to_double()); }
};

// cos
struct cos_impl
{
    value_type operator()(value_type const& val) const { return std::cos(val.to_double()); }
};

// tan
struct tan_impl
{
    value_type operator()(value_type const& val) const { return std::tan(val.to_double()); }
};

// atan
struct atan_impl
{
    value_type operator()(value_type const& val) const { return std::atan(val.to_double()); }
};

// abs
struct abs_impl
{
    value_type operator()(value_type const& val) const { return std::abs(val.to_double()); }
};

// length
struct length_impl
{
    value_type operator()(value_type const& val) const { return val.to_unicode().length(); }
};

// min
inline value_type min_impl(value_type const& arg1, value_type const& arg2)
{
    return std::min(arg1.to_double(), arg2.to_double());
}

// max
inline value_type max_impl(value_type const& arg1, value_type const& arg2)
{
    return std::max(arg1.to_double(), arg2.to_double());
}

// pow
inline value_type pow_impl(value_type const& arg1, value_type const& arg2)
{
    return std::pow(arg1.to_double(), arg2.to_double());
}

} // namespace mapnik

#endif // MAPNIK_FUNCTION_CALL_HPP
