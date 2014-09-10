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
//template <typename T>
struct exp_impl
{
    //using type = T;
    value_type operator() (value_type const& val) const
    {
        return std::exp(val.to_double());
    }

};

// sin
struct sin_impl
{
    value_type operator() (value_type const& val) const
    {
        return std::sin(val.to_double());
    }
};

// cos
struct cos_impl
{
    value_type operator() (value_type const& val) const
    {
        return std::cos(val.to_double());
    }
};

// tan
struct tan_impl
{
    value_type operator() (value_type const& val) const
    {
        return std::tan(val.to_double());
    }
};

// atan
struct atan_impl
{
    value_type operator()(value_type const& val) const
    {
        return std::atan(val.to_double());
    }
};

// abs
struct abs_impl
{
    value_type operator() (value_type const& val) const
    {
        return std::fabs(val.to_double());
    }
};

// length
struct length_impl
{
    value_type operator() (value_type const& val) const
    {
        return val.to_unicode().length();
    }
};

unary_function_types::unary_function_types()
{
    add
        ("sin",  sin_impl())
        ("cos",  cos_impl())
        ("tan",  tan_impl())
        ("atan", atan_impl())
        ("exp",  exp_impl())
        ("abs",  abs_impl())
        ("length",length_impl())
        ;
}

char const* unary_function_name(unary_function_impl const& fun)
{
    if (fun.target<sin_impl>()) return "sin";
    else if (fun.target<cos_impl>()) return "cos";
    else if (fun.target<tan_impl>()) return "tan";
    else if (fun.target<atan_impl>()) return "atan";
    else if (fun.target<exp_impl>()) return "exp";
    else if (fun.target<abs_impl>()) return "abs";
    else return "unknown";
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
    return "unknown";
}

}
