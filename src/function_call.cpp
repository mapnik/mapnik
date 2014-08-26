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

#include <mapnik/function_call.hpp>

namespace mapnik {


// functions
// exp
inline value_type exp_impl (value_type const& val)
{
    return std::exp(val.to_double());
}
// sin
inline value_type sin_impl (value_type const& val)
{
    return std::sin(val.to_double());
}
// cos
inline value_type cos_impl (value_type const& val)
{
    return std::cos(val.to_double());
}
// tan
inline value_type tan_impl (value_type const& val)
{
    return std::tan(val.to_double());
}
// atan
inline value_type atan_impl (value_type const& val)
{
    return std::atan(val.to_double());
}
// abs
inline value_type abs_impl (value_type const& val)
{
    return std::fabs(val.to_double());
}

unary_function_types::unary_function_types()
{
    add
        ("sin", unary_function_impl(sin_impl))
        ("cos", unary_function_impl(cos_impl))
        ("tan", unary_function_impl(tan_impl))
        ("atan", unary_function_impl(atan_impl))
        ("exp", unary_function_impl(exp_impl))
        ("abs", unary_function_impl(abs_impl))
        ;
}

char const* unary_function_name(unary_function_impl const& fun)
{
    value_type(*const* f_ptr)(value_type const&) = fun.target<value_type(*)(value_type const&)>();

    if (f_ptr)
    {
        if (*f_ptr == sin_impl) return "sin";
        else if(*f_ptr == cos_impl) return "cos";
        else if(*f_ptr == tan_impl) return "tan";
        else if(*f_ptr == atan_impl) return "atan";
        else if(*f_ptr == exp_impl) return "exp";
        else if(*f_ptr == abs_impl) return "abs";
    }
    return "";
}

// binary functions
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

binary_function_types::binary_function_types()
{
    add
        ("min", binary_function_impl(min_impl))
        ("max", binary_function_impl(max_impl))
        ("pow", binary_function_impl(pow_impl))
        ;
}

char const* binary_function_name(binary_function_impl const& fun)
{
    value_type(*const* f_ptr)(value_type const&, value_type const&) =
        fun.target<value_type(*)
                   (value_type const&, value_type const&)>();
    if (f_ptr)
    {
        if (*f_ptr == min_impl) return "min";
        else if(*f_ptr == max_impl) return "max";
        else if(*f_ptr == pow_impl) return "pow";
    }
    return "";
}

}
