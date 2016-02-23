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

#ifndef MAPNIK_VALUE_HPP
#define MAPNIK_VALUE_HPP

// mapnik
#include <mapnik/value_types.hpp>
#include <mapnik/value_hash.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/variant.hpp>

// stl
#include <string>
#include <cmath>
#include <memory>

#include <iosfwd>
#include <cstddef>
#include <new>
#include <type_traits>

// icu
#include <unicode/unistr.h>
#include <unicode/ustring.h>

namespace mapnik {

using value_base = util::variant<value_null, value_bool, value_integer,value_double, value_unicode_string>;

inline void to_utf8(mapnik::value_unicode_string const& input, std::string & target)
{
    target.clear(); // mimic previous target.assign(...) semantics
    input.toUTF8String(target); // this appends to target
}

namespace detail {

namespace {
template <typename T, typename U>
struct both_arithmetic : std::integral_constant<bool,
                                             std::is_arithmetic<T>::value &&
                                             std::is_arithmetic<U>::value > {};

struct equals
{
    static bool apply(value_null, value_unicode_string const& rhs)
    {
        return false;
    }

    template <typename T>
    static auto apply(T const& lhs, T const& rhs)
        -> decltype(lhs == rhs)
    {
        return lhs == rhs;
    }
};

struct not_equal
{
    // back compatibility shim to equate empty string with null for != test
    // https://github.com/mapnik/mapnik/issues/1859
    // TODO - consider removing entire specialization at Mapnik 3.1.x
    static bool apply(value_null, value_unicode_string const& rhs)
    {
        if (rhs.isEmpty()) return false;
        return true;
    }

    template <typename T>
    static auto apply(T const& lhs, T const& rhs)
        ->decltype(lhs != rhs)
    {
        return lhs != rhs;
    }
};

struct greater_than
{
    static bool apply(value_null, value_unicode_string const& rhs)
    {
        return false;
    }

    template <typename T>
    static auto  apply(T const& lhs, T const& rhs)
        ->decltype(lhs > rhs)
    {
        return lhs > rhs;
    }
};

struct greater_or_equal
{
    static bool apply(value_null, value_unicode_string const& rhs)
    {
        return false;
    }

    template <typename T>
    static auto apply(T const& lhs, T const& rhs)
        ->decltype(lhs >= rhs)
    {
        return lhs >= rhs;
    }
};

struct less_than
{
    static bool apply(value_null, value_unicode_string const& rhs)
    {
        return false;
    }

    template <typename T>
    static auto apply(T const& lhs, T const& rhs)
         ->decltype(lhs < rhs)
    {
        return lhs < rhs;
    }
};

struct less_or_equal
{
    static bool apply(value_null, value_unicode_string const& rhs)
    {
        return false;
    }

    template <typename T>
    static auto apply(T const& lhs, T const& rhs)
        ->decltype(lhs <= rhs)
    {
        return lhs <= rhs;
    }
};

}

template <typename Op, bool default_result>
struct comparison
{
    // special case for unicode_strings (fixes MSVC C4800)
    bool operator() (value_unicode_string const& lhs,
                     value_unicode_string const& rhs) const
    {
        return Op::apply(lhs, rhs) ? true : false;
    }

    //////////////////////////////////////////////////////////////////////////
    // special case for unicode_string and value_null
    //////////////////////////////////////////////////////////////////////////

    bool operator() (value_null const& lhs, value_unicode_string const& rhs) const
    {
        return Op::apply(lhs, rhs);
    }
    //////////////////////////////////////////////////////////////////////////


    // same types
    template <typename T>
    bool operator() (T lhs, T rhs) const
    {
        return Op::apply(lhs, rhs);
    }

    // both types are arithmetic - promote to the common type
    template <typename T, typename U, typename std::enable_if<both_arithmetic<T,U>::value, int>::type = 0>
    bool operator() (T const& lhs, U const& rhs) const
    {
        using common_type = typename std::common_type<T,U>::type;
        return Op::apply(static_cast<common_type>(lhs),static_cast<common_type>(rhs));
    }

    //
    template <typename T, typename U, typename std::enable_if<!both_arithmetic<T,U>::value, int>::type = 0>
    bool operator() (T const& lhs, U const& rhs) const
    {
        return default_result;
    }
};

template <typename V>
struct add
{
    using value_type = V;
    value_type operator() (value_unicode_string const& lhs ,
                           value_unicode_string const& rhs ) const
    {
        return lhs + rhs;
    }

    value_type operator() (value_null const& lhs ,
                           value_null const& rhs) const
    {
        return lhs;
    }

    value_type operator() (value_unicode_string const& lhs, value_null) const
    {
        return lhs;
    }

    value_type operator() (value_null, value_unicode_string const& rhs) const
    {
        return rhs;
    }

    template <typename L>
    value_type operator() (L const& lhs, value_null const&) const
    {
        return lhs;
    }

    template <typename R>
    value_type operator() (value_null const&, R const& rhs) const
    {
        return rhs;
    }

    template <typename L>
    value_type operator() (L const& lhs , value_unicode_string const& rhs) const
    {
        std::string val;
        if (util::to_string(val,lhs))
            return value_unicode_string(val.c_str()) + rhs;
        return rhs;
    }

    template <typename R>
    value_type operator() (value_unicode_string const& lhs, R const& rhs) const
    {
        std::string val;
        if (util::to_string(val,rhs))
            return lhs + value_unicode_string(val.c_str());
        return lhs;
    }

    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs, T2 const& rhs) const
    {
        return typename std::common_type<T1,T2>::type{ lhs + rhs };
    }

    value_type operator() (value_bool lhs, value_bool rhs) const
    {
        return value_integer(lhs + rhs);
    }
};

template <typename V>
struct sub
{
    using value_type = V;

    value_type operator() (value_null const& lhs ,
                           value_null const& rhs) const
    {
        return lhs;
    }

    value_type operator() (value_null, value_unicode_string const& rhs) const
    {
        return rhs;
    }
    value_type operator() (value_unicode_string const& lhs, value_null) const
    {
        return lhs;
    }

    template <typename R>
    value_type operator() (value_unicode_string const& lhs, R const&) const
    {
        return lhs;
    }

    template <typename L>
    value_type operator() (L const&, value_unicode_string const& rhs) const
    {
        return rhs;
    }

    template <typename L>
    value_type operator() (L const& lhs, value_null const&) const
    {
        return lhs;
    }

    template <typename R>
    value_type operator() (value_null const&, R const& rhs) const
    {
        return rhs;
    }

    template <typename T>
    value_type operator() (T lhs, T rhs) const
    {
        return lhs - rhs ;
    }

    value_type operator() (value_unicode_string const&,
                           value_unicode_string const&) const
    {
        return value_type();
    }

    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs, T2 const& rhs) const
    {
        return typename std::common_type<T1,T2>::type{ lhs - rhs };
    }


    value_type operator() (value_bool lhs, value_bool rhs) const
    {
        return value_integer(lhs - rhs);
    }
};

template <typename V>
struct mult
{
    using value_type = V;

    value_type operator() (value_null const& lhs ,
                           value_null const& rhs) const
    {
        return lhs;
    }

    value_type operator() (value_unicode_string const& lhs, value_null) const
    {
        return lhs;
    }

    value_type operator() (value_null, value_unicode_string const& rhs) const
    {
        return rhs;
    }

    template <typename L>
    value_type operator() (L const& lhs, value_null const&) const
    {
        return lhs;
    }

    template <typename R>
    value_type operator() (value_null const&, R const& rhs) const
    {
        return rhs;
    }

    template <typename R>
    value_type operator() (value_unicode_string const& lhs, R const&) const
    {
        return lhs;
    }

    template <typename L>
    value_type operator() (L const&, value_unicode_string const& rhs) const
    {
        return rhs;
    }

    template <typename T>
    value_type operator() (T lhs, T rhs) const
    {
        return lhs * rhs;
    }

    value_type operator() (value_unicode_string const&,
                           value_unicode_string const&) const
    {
        return value_type();
    }

    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs, T2 const& rhs) const
    {
        return typename std::common_type<T1,T2>::type{ lhs * rhs };
    }

    value_type operator() (value_bool lhs, value_bool rhs) const
    {
        return value_integer(lhs * rhs);
    }
};

template <typename V>
struct div
{
    using value_type = V;

    value_type operator() (value_null const& lhs ,
                           value_null const& rhs) const
    {
        return lhs;
    }

    value_type operator() (value_unicode_string const& lhs, value_null) const
    {
        return lhs;
    }

    value_type operator() (value_null, value_unicode_string const& rhs) const
    {
        return rhs;
    }

    template <typename L>
    value_type operator() (L const& lhs, value_null const&) const
    {
        return lhs;
    }

    template <typename R>
    value_type operator() (value_null const&, R const& rhs) const
    {
        return rhs;
    }

    template <typename T>
    value_type operator() (T lhs, T rhs) const
    {
        if (rhs == 0) return value_type();
        return lhs / rhs;
    }

    value_type operator() (value_bool lhs, value_bool rhs) const
    {
        if (rhs == 0) return lhs;
        return value_integer(lhs) / value_integer(rhs);
    }

    value_type operator() (value_unicode_string const&,
                           value_unicode_string const&) const
    {
        return value_type();
    }

    template <typename R>
    value_type operator() (value_unicode_string const& lhs, R const&) const
    {
        return lhs;
    }

    template <typename L>
    value_type operator() (L const&, value_unicode_string const& rhs) const
    {
        return rhs;
    }

    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs, T2 const& rhs) const
    {
        if (rhs == 0) return value_type();
        using common_type = typename std::common_type<T1,T2>::type;
        return common_type(lhs)/common_type(rhs);
    }
};

template <typename V>
struct mod
{
    using value_type = V;

    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs, T2 const&) const
    {
       return lhs;
    }

    template <typename T>
    value_type operator() (T lhs, T rhs) const
    {
        return lhs % rhs;
    }

    value_type operator() (value_unicode_string const&,
                           value_unicode_string const&) const
    {
        return value_type();
    }

    value_type operator() (value_bool,
                           value_bool) const
    {
        return false;
    }

    value_type operator() (value_double lhs, value_integer rhs) const
    {
        return std::fmod(lhs, static_cast<value_double>(rhs));
    }

    value_type operator() (value_integer lhs, value_double rhs) const
    {
        return std::fmod(static_cast<value_double>(lhs), rhs);
    }

    value_type operator() (value_double lhs, value_double rhs) const
    {
        return std::fmod(lhs, rhs);
    }
};

template <typename V>
struct negate
{
    using value_type = V;

    template <typename T>
    value_type operator() (T val) const
    {
        return -val;
    }

    value_type operator() (value_null val) const
    {
        return val;
    }

    value_type operator() (value_bool val) const
    {
        return val ? value_integer(-1) : value_integer(0);
    }

    value_type operator() (value_unicode_string const&) const
    {
        return value_type();
    }
};

// converters
template <typename T>
struct convert {};

template <>
struct convert<value_bool>
{
    value_bool operator() (value_bool val) const
    {
        return val;
    }

    value_bool operator() (value_unicode_string const& ustr) const
    {
        return !ustr.isEmpty();
    }

    value_bool operator() (value_null const&) const
    {
        return false;
    }

    template <typename T>
    value_bool operator() (T val) const
    {
        return val > 0 ? true : false;
    }
};

template <>
struct convert<value_double>
{
    value_double operator() (value_double val) const
    {
        return val;
    }

    value_double operator() (value_integer val) const
    {
        return static_cast<value_double>(val);
    }

    value_double operator() (value_bool val) const
    {
        return static_cast<value_double>(val);
    }

    value_double operator() (std::string const& val) const
    {
        value_double result;
        if (util::string2double(val,result))
            return result;
        return 0;
    }

    value_double operator() (value_unicode_string const& val) const
    {
        std::string utf8;
        val.toUTF8String(utf8);
        return operator()(utf8);
    }

    value_double operator() (value_null const&) const
    {
        return 0.0;
    }
};

template <>
struct convert<value_integer>
{
    value_integer operator() (value_integer val) const
    {
        return val;
    }

    value_integer operator() (value_double val) const
    {
        return static_cast<value_integer>(rint(val));
    }

    value_integer operator() (value_bool val) const
    {
        return static_cast<value_integer>(val);
    }

    value_integer operator() (std::string const& val) const
    {
        value_integer result;
        if (util::string2int(val,result))
            return result;
        return value_integer(0);
    }

    value_integer operator() (value_unicode_string const& val) const
    {
        std::string utf8;
        val.toUTF8String(utf8);
        return operator()(utf8);
    }

    value_integer operator() (value_null const&) const
    {
        return value_integer(0);
    }
};

template <>
struct convert<std::string>
{
    template <typename T>
    std::string operator() (T val) const
    {
        std::string str;
        util::to_string(str, val);
        return str;
    }

    // specializations
    std::string operator() (value_unicode_string const& val) const
    {
        std::string utf8;
        val.toUTF8String(utf8);
        return utf8;
    }

    std::string operator() (value_double val) const
    {
        std::string str;
        util::to_string(str, val); // TODO set precision(16)
        return str;
    }

    std::string operator() (value_bool val) const
    {
        return val ? "true": "false";
    }

    std::string operator() (value_null const&) const
    {
        return std::string();
    }
};

struct to_unicode_impl
{

    template <typename T>
    value_unicode_string operator() (T val) const
    {
        std::string str;
        util::to_string(str,val);
        return value_unicode_string(str.c_str());
    }

    // specializations
    value_unicode_string const& operator() (value_unicode_string const& val) const
    {
        return val;
    }

    value_unicode_string operator() (value_double val) const
    {
        std::string str;
        util::to_string(str,val);
        return value_unicode_string(str.c_str());
    }

    value_unicode_string operator() (value_bool val) const
    {
        return value_unicode_string(val ? "true" : "false");
    }

    value_unicode_string operator() (value_null const&) const
    {
        return value_unicode_string();
    }
};

struct to_expression_string_impl
{
    struct EscapingByteSink : U_NAMESPACE_QUALIFIER ByteSink
    {
        std::string dest_;
        char quote_;

        explicit EscapingByteSink(char quote)
            : quote_(quote)
        {}

        virtual void Append(const char* data, int32_t n)
        {
            // reserve enough room to hold the appended chunk and quotes;
            // if another chunk follows, or any character needs escaping,
            // the string will grow naturally
            if (dest_.empty())
            {
                dest_.reserve(2 + static_cast<std::size_t>(n));
                dest_.append(1, quote_);
            }
            else
            {
                dest_.reserve(dest_.size() + n + 1);
            }

            for (auto end = data + n; data < end; ++data)
            {
                if (*data == '\\' || *data == quote_)
                    dest_.append(1, '\\');
                dest_.append(1, *data);
            }
        }

        virtual void Flush()
        {
            if (dest_.empty())
                dest_.append(2, quote_);
            else
                dest_.append(1, quote_);
        }
    };

    explicit to_expression_string_impl(char quote = '\'')
        : quote_(quote) {}

    std::string operator() (value_unicode_string const& val) const
    {
        EscapingByteSink sink(quote_);
        val.toUTF8(sink);
        return sink.dest_;
    }

    std::string operator() (value_integer val) const
    {
        std::string output;
        util::to_string(output,val);
        return output;
    }

    std::string operator() (value_double val) const
    {
        std::string output;
        util::to_string(output,val); // TODO precision(16)
        return output;
    }

    std::string operator() (value_bool val) const
    {
        return val ? "true" : "false";
    }

    std::string operator() (value_null const&) const
    {
        return "null";
    }

    const char quote_;
};


} // namespace detail

namespace value_adl_barrier {

class value : public value_base
{
    friend const value operator+(value const&,value const&);
    friend const value operator-(value const&,value const&);
    friend const value operator*(value const&,value const&);
    friend const value operator/(value const&,value const&);
    friend const value operator%(value const&,value const&);

public:
    value() = default;

    // conversion from type T is done via a temporary of type U, which
    // is determined by mapnik_value_type;
    // enable_if< decay<T> != value > is necessary to avoid ill-formed
    // recursion in noexcept specifier; and it also prevents using this
    // constructor where implicitly-declared copy/move should be used
    // (e.g. value(value&))
    template <typename T,
              typename U = typename std::enable_if<
                                !detail::is_same_decay<T, value>::value,
                                detail::mapnik_value_type_decay<T>
                            >::type::type>
    value(T && val)
        noexcept(noexcept(U(std::forward<T>(val))) &&
                 std::is_nothrow_constructible<value_base, U && >::value)
        : value_base(U(std::forward<T>(val))) {}

    template <typename T,
              typename U = typename std::enable_if<
                                !detail::is_same_decay<T, value>::value,
                                detail::mapnik_value_type_decay<T>
                            >::type::type>
    value& operator=(T && val)
        noexcept(noexcept(U(std::forward<T>(val))) &&
                 std::is_nothrow_assignable<value_base, U && >::value)
    {
        value_base::operator=(U(std::forward<T>(val)));
        return *this;
    }

    bool operator==(value const& other) const
    {
        return util::apply_visitor(detail::comparison<detail::equals, false>(), *this, other);
    }

    bool operator!=(value const& other) const
    {
        return util::apply_visitor(detail::comparison<detail::not_equal, true>(), *this, other);
    }

    bool operator>(value const& other) const
    {
        return util::apply_visitor(detail::comparison<detail::greater_than, false>(), *this, other);
    }

    bool operator>=(value const& other) const
    {
        return util::apply_visitor(detail::comparison<detail::greater_or_equal, false>(), *this, other);
    }

    bool operator<(value const& other) const
    {
        return util::apply_visitor(detail::comparison<detail::less_than, false>(), *this, other);
    }

    bool operator<=(value const& other) const
    {
        return util::apply_visitor(detail::comparison<detail::less_or_equal, false>(), *this, other);
    }

    value operator- () const
    {
        return util::apply_visitor(detail::negate<value>(), *this);
    }

    bool is_null() const;

    template <typename T>
    T convert() const
    {
        return util::apply_visitor(detail::convert<T>(),*this);
    }

    value_bool to_bool() const
    {
        return util::apply_visitor(detail::convert<value_bool>(),*this);
    }

    std::string to_expression_string(char quote = '\'') const
    {
        return util::apply_visitor(detail::to_expression_string_impl(quote),*this);
    }

    std::string to_string() const
    {
        return util::apply_visitor(detail::convert<std::string>(),*this);
    }

    value_unicode_string to_unicode() const
    {
        return util::apply_visitor(detail::to_unicode_impl(),*this);
    }

    value_double to_double() const
    {
        return util::apply_visitor(detail::convert<value_double>(),*this);
    }

    value_integer to_int() const
    {
        return util::apply_visitor(detail::convert<value_integer>(),*this);
    }
};

inline const value operator+(value const& p1,value const& p2)
{
    return value(util::apply_visitor(detail::add<value>(),p1, p2));
}

inline const value operator-(value const& p1,value const& p2)
{
    return value(util::apply_visitor(detail::sub<value>(),p1, p2));
}

inline const value operator*(value const& p1,value const& p2)
{
    return value(util::apply_visitor(detail::mult<value>(),p1, p2));
}

inline const value operator/(value const& p1,value const& p2)
{
    return value(util::apply_visitor(detail::div<value>(),p1, p2));
}

inline const value operator%(value const& p1,value const& p2)
{
    return value(util::apply_visitor(detail::mod<value>(),p1, p2));
}

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
    return mapnik_hash_value(val);
}

} // namespace value_adl_barrier

using value_adl_barrier::value;

namespace detail {

struct is_null_visitor
{
    bool operator() (value const& val) const
    {
        return val.is_null();
    }

    bool operator() (value_null const&) const
    {
        return true;
    }

    template <typename T>
    bool operator() (T const&) const
    {
        return false;
    }
};

} // namespace detail

inline bool value::is_null() const
{
    return util::apply_visitor(mapnik::detail::is_null_visitor(), *this);
}

} // namespace mapnik

// support for std::unordered_xxx
namespace std
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-tags"

template <>
struct hash<mapnik::value>
{
    size_t operator()(mapnik::value const& val) const
    {
        return mapnik::mapnik_hash_value(val);
    }
};

#pragma GCC diagnostic pop

}

#endif // MAPNIK_VALUE_HPP
