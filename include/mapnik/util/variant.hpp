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

#define USE_BOOST

#ifdef USE_BOOST
    #include <boost/variant.hpp>
    #define VARIANT_INLINE inline
#else
    #include <mapbox/variant.hpp>
#endif

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/mpl/vector.hpp> // spirit support
#pragma GCC diagnostic pop

namespace mapnik { namespace util {

#ifdef USE_BOOST

template <typename T>
using recursive_wrapper = typename boost::recursive_wrapper<T>;

template<typename... Types>
using variant = boost::variant<Types...>;

template <typename T>
using static_visitor = boost::static_visitor<T>;

// unary visitor interface
// const
template <typename F, typename V>
auto VARIANT_INLINE static apply_visitor(F && f, V const& v) -> decltype(boost::apply_visitor(std::forward<F>(f),v))
{
    return boost::apply_visitor(std::forward<F>(f),v);
}
// non-const
template <typename F, typename V>
auto VARIANT_INLINE static apply_visitor(F && f, V & v) -> decltype(boost::apply_visitor(std::forward<F>(f),v))
{
    return boost::apply_visitor(std::forward<F>(f),v);
}

// binary visitor interface
// const
template <typename F, typename V>
auto VARIANT_INLINE static apply_visitor(F && f, V const& v0, V const& v1) -> decltype(boost::apply_visitor(std::forward<F>(f), v0, v1))
{
    return boost::apply_visitor(std::forward<F>(f), v0, v1);
}

// non-const
template <typename F, typename V>
auto VARIANT_INLINE static apply_visitor(F && f, V & v0, V & v1) -> decltype(boost::apply_visitor(std::forward<F>(f), v0, v1))
{
    return boost::apply_visitor(std::forward<F>(f), v0, v1);
}

// getter interface

template <typename ResultType, typename T>
auto get(T & var)->decltype(boost::get<ResultType>(var))
{
    return boost::get<ResultType>(var);
}

template <typename ResultType, typename T>
auto get(T const& var)->decltype(boost::get<ResultType>(var))
{
    return boost::get<ResultType>(var);
}

#else

template <typename T>
using recursive_wrapper = typename mapbox::util::recursive_wrapper<T>;

template <typename T>
using static_visitor = mapbox::util::static_visitor<T>;

template<typename... Types>
class variant : public mapbox::util::variant<Types...>
{
public:
    // tell spirit that this is an adapted variant
    struct adapted_variant_tag;
    using types = boost::mpl::vector<Types...>;
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
auto get(T& var)->decltype(var.template get<ResultType>())
{
    return var.template get<ResultType>();
}

template <typename ResultType, typename T>
auto get(T const& var)->decltype(var.template get<ResultType>())
{
    return var.template get<ResultType>();
}

#endif




}}

#endif  // MAPNIK_UTIL_VARIANT_HPP
