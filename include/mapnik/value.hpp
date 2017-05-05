/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_VALUE_HPP
#define MAPNIK_VALUE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/value/hash.hpp>
#include <mapnik/util/variant.hpp>


namespace mapnik {

using value_base = util::variant<value_null, value_bool, value_integer,value_double, value_unicode_string>;

namespace value_adl_barrier {

class MAPNIK_DECL value : public value_base
{
    friend MAPNIK_DECL value operator+(value const&,value const&);
    friend MAPNIK_DECL value operator-(value const&,value const&);
    friend MAPNIK_DECL value operator*(value const&,value const&);
    friend MAPNIK_DECL value operator/(value const&,value const&);
    friend MAPNIK_DECL value operator%(value const&,value const&);

public:
    value() = default;

    // Conversion from type T is done via a temporary value or reference
    // of type U, which is determined by mapnik_value_type_t.
    //
    // CAVEAT: We don't check `noexcept(conversion from T to U)`.
    //         But since the type U is either value_bool, value_integer,
    //         value_double or T &&, this conversion SHOULD NEVER throw.
    template <typename T, typename U = detail::mapnik_value_type_t<T>>
    value(T && val)
        noexcept(std::is_nothrow_constructible<value_base, U>::value)
        : value_base(U(std::forward<T>(val))) {}

    template <typename T, typename U = detail::mapnik_value_type_t<T>>
    value& operator=(T && val)
        noexcept(std::is_nothrow_assignable<value_base, U>::value)
    {
        value_base::operator=(U(std::forward<T>(val)));
        return *this;
    }

    bool operator==(value const& other) const;
    bool operator!=(value const& other) const;
    bool operator>(value const& other) const;
    bool operator>=(value const& other) const;
    bool operator<(value const& other) const;
    bool operator<=(value const& other) const;

    value operator-() const;

    bool is_null() const;

    template <typename T> T convert() const;

    value_bool to_bool() const;
    std::string to_expression_string(char quote = '\'') const;
    std::string to_string() const;
    value_unicode_string to_unicode() const;
    value_double to_double() const;
    value_integer to_int() const;
};

MAPNIK_DECL value operator+(value const& p1,value const& p2);
MAPNIK_DECL value operator-(value const& p1,value const& p2);
MAPNIK_DECL value operator*(value const& p1,value const& p2);
MAPNIK_DECL value operator/(value const& p1,value const& p2);
MAPNIK_DECL value operator%(value const& p1,value const& p2);

template <typename charT, typename traits>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             value const& v)
{
    out << v.to_string();
    return out;
}

// hash function
inline std::size_t hash_value(value const& val)
{
    return mapnik::value_hash(val);
}

} // namespace value_adl_barrier

using value = value_adl_barrier::value;

namespace detail {
struct is_null_visitor
{
    bool operator()(value const& val) const
    {
        return val.is_null();
    }

    bool operator()(value_null const&) const
    {
        return true;
    }

    template <typename T>
    bool operator()(T const&) const
    {
        return false;
    }
};
} // namespace detail
} // namespace mapnik

// support for std::unordered_xxx
namespace std
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wmismatched-tags"

template <>
struct hash<mapnik::value>
{
    size_t operator()(mapnik::value const& val) const
    {
        return mapnik::value_hash(val);
    }
};

#pragma GCC diagnostic pop

}

#endif // MAPNIK_VALUE_HPP
