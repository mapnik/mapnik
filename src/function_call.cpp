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

#include <mapnik/function_call.hpp>

namespace mapnik {

char const* unary_function_name(unary_function_impl const& fun)
{
    if (fun.target<sin_impl>())
        return "sin";
    else if (fun.target<cos_impl>())
        return "cos";
    else if (fun.target<tan_impl>())
        return "tan";
    else if (fun.target<atan_impl>())
        return "atan";
    else if (fun.target<exp_impl>())
        return "exp";
    else if (fun.target<abs_impl>())
        return "abs";
    else if (fun.target<length_impl>())
        return "length";
    else
        return "unknown";
}

// binary functions
char const* binary_function_name(binary_function_impl const& fun)
{
    value_type (*const* f_ptr)(value_type const&, value_type const&) =
      fun.target<value_type (*)(value_type const&, value_type const&)>();
    if (f_ptr)
    {
        if (*f_ptr == min_impl)
            return "min";
        else if (*f_ptr == max_impl)
            return "max";
        else if (*f_ptr == pow_impl)
            return "pow";
    }
    return "unknown";
}

} // namespace mapnik
