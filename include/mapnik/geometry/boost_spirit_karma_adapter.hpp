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


#ifndef MAPNIK_BOOST_SPIRIT_KARMA_ADAPTER_HPP
#define MAPNIK_BOOST_SPIRIT_KARMA_ADAPTER_HPP

#include <mapnik/geometry.hpp>
#include <cstdint>

namespace boost { using mapbox::util::get; }

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/support/attributes.hpp>

namespace boost { namespace spirit { namespace traits
{
template <>
struct not_is_variant<mapnik::geometry::geometry<double>, karma::domain>
    : mpl::false_
{};

template <>
struct not_is_variant<mapnik::geometry::geometry<std::int64_t>, karma::domain>
    : mpl::false_
{};

template <>
struct variant_which< mapnik::geometry::geometry<double> >
{
    static int call(mapnik::geometry::geometry<double> const& v)
    {
        return v.which();
    }
};

template <>
struct variant_which< mapnik::geometry::geometry<std::int64_t> >
{
    static int call(mapnik::geometry::geometry<std::int64_t> const& v)
    {
        return v.which();
    }
};

namespace detail {

template <typename T, typename Tuple>
struct has_type;

template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Types>
struct has_type<T, std::tuple<U, Types...>> : has_type<T, std::tuple<Types...>> {};

template <typename T, typename... Types>
struct has_type<T, std::tuple<T, Types...>> : std::true_type {};

template <typename T, typename Tuple>
struct index;

template <typename T, typename... Types>
struct index<T, std::tuple<T, Types...>>
{
    static const std::size_t value = 0;
};

template <typename T, typename U, typename... Types>
struct index<T, std::tuple<U, Types...>>
{
    static const std::size_t value = 1 + index<T, std::tuple<Types...>>::value;
};

}

template <typename Expected>
struct compute_compatible_component_variant<mapnik::geometry::geometry<double>, Expected>
    :  detail::has_type<Expected, mapnik::geometry::geometry<double>::types>
{
    using compatible_type = Expected;
    static bool is_compatible(int index)
    {
        return (index == detail::index<compatible_type, mapnik::geometry::geometry<double>::types>::value);
    }
};

template <typename Expected>
struct compute_compatible_component_variant<mapnik::geometry::geometry<std::int64_t>, Expected>
    :  detail::has_type<Expected, mapnik::geometry::geometry<std::int64_t>::types>
{
    using compatible_type = Expected;
    static bool is_compatible(int index)
    {
        return (index == detail::index<compatible_type, mapnik::geometry::geometry<std::int64_t>::types>::value);
    }
};

}}}



#endif //MAPNIK_BOOST_SPIRIT_KARMA_ADAPTER_HPP
