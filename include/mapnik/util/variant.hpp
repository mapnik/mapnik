/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/adapted/std_tuple.hpp> // spirit support
#pragma GCC diagnostic pop

namespace mapnik { namespace util {

template <typename T>
using recursive_wrapper = typename mapbox::util::recursive_wrapper<T>;

template<typename... Types>
class variant : public mapbox::util::variant<Types...>
{
public:
    // tell spirit that this is an adapted variant
    struct adapted_variant_tag;
    using types = std::tuple<Types...>;
    // inherit ctor's
    using mapbox::util::variant<Types...>::variant;
};

// unary visitor interface
// const
template <typename F, typename V>
auto VARIANT_INLINE static apply_visitor(F && f, V const& v) -> decltype(V::visit(v, std::forward<F>(f)))
{
    return V::visit(v, std::forward<F>(f));
}
// non-const
template <typename F, typename V>
auto VARIANT_INLINE static apply_visitor(F && f, V & v) -> decltype(V::visit(v, std::forward<F>(f)))
{
    return V::visit(v, std::forward<F>(f));
}

// binary visitor interface
// const
template <typename F, typename V>
auto VARIANT_INLINE static apply_visitor(F && f, V const& v0, V const& v1) -> decltype(V::binary_visit(v0, v1, std::forward<F>(f)))
{
    return V::binary_visit(v0, v1, std::forward<F>(f));
}

// non-const
template <typename F, typename V>
auto VARIANT_INLINE static apply_visitor(F && f, V & v0, V & v1) -> decltype(V::binary_visit(v0, v1, std::forward<F>(f)))
{
    return V::binary_visit(v0, v1, std::forward<F>(f));
}

// getter interface
template <typename ResultType, typename T>
ResultType & get(T & var)
{
    return var.template get<ResultType>();
}

template <typename ResultType, typename T>
ResultType const& get(T const& var)
{
    return var.template get<ResultType>();
}

}}

#endif  // MAPNIK_UTIL_VARIANT_HPP
