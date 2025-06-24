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

#ifndef MAPNIK_UTIL_VARIANT_HPP
#define MAPNIK_UTIL_VARIANT_HPP

#include <mapnik/config.hpp>
#include <mapbox/variant.hpp>

namespace mapnik {
namespace util {

template<typename T>
using recursive_wrapper = typename mapbox::util::recursive_wrapper<T>;

template<typename... Types>
using variant = typename mapbox::util::variant<Types...>;

// unary visitor interface
template<typename F, typename V>
auto VARIANT_INLINE apply_visitor(F&& f, V&& v) -> decltype(v.visit(std::forward<V>(v), std::forward<F>(f)))
{
    return v.visit(std::forward<V>(v), std::forward<F>(f));
}

// binary visitor interface
template<typename F, typename V>
auto VARIANT_INLINE apply_visitor(F&& f, V&& v0, V&& v1)
  -> decltype(v0.binary_visit(std::forward<V>(v0), std::forward<V>(v1), std::forward<F>(f)))
{
    return v0.binary_visit(std::forward<V>(v0), std::forward<V>(v1), std::forward<F>(f));
}

// getter interface
template<typename ResultType, typename T>
ResultType& get(T& var)
{
    return var.template get<ResultType>();
}

template<typename ResultType, typename T>
ResultType const& get(T const& var)
{
    return var.template get<ResultType>();
}

} // namespace util
} // namespace mapnik

#endif // MAPNIK_UTIL_VARIANT_HPP
