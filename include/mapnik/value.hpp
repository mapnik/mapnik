/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/global.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/conversions.hpp>

// boost
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>

#include <boost/concept_check.hpp>
#include <boost/functional/hash.hpp>
#include "hash_variant.hpp"

// stl
#include <string>
#include <cmath>
#include <memory>

// icu
#include <unicode/unistr.h>
#include <unicode/ustring.h>

namespace mapnik  {

inline void to_utf8(mapnik::value_unicode_string const& input, std::string & target)
{
    if (input.isEmpty()) return;

    const int BUF_SIZE = 256;
    char buf [BUF_SIZE];
    int len;

    UErrorCode err = U_ZERO_ERROR;
    u_strToUTF8(buf, BUF_SIZE, &len, input.getBuffer(), input.length(), &err);
    if (err == U_BUFFER_OVERFLOW_ERROR || err == U_STRING_NOT_TERMINATED_WARNING )
    {
        const std::unique_ptr<char[]> buf_ptr(new char [len+1]);
        err = U_ZERO_ERROR;
        u_strToUTF8(buf_ptr.get() , len + 1, &len, input.getBuffer(), input.length(), &err);
        target.assign(buf_ptr.get() , static_cast<std::size_t>(len));
    }
    else
    {
        target.assign(buf, static_cast<std::size_t>(len));
    }
}

typedef boost::variant<value_null,value_bool,value_integer,value_double,value_unicode_string> value_base;

namespace impl {

struct equals
    : public boost::static_visitor<bool>
{
    bool operator() (value_integer lhs, value_double rhs) const
    {
        return  lhs == rhs;
    }

    bool operator() (value_bool lhs, value_double rhs) const
    {
        return  lhs == rhs;
    }

    bool operator() (value_double lhs, value_integer rhs) const
    {
        return  lhs == rhs;
    }

    bool operator() (value_bool lhs, value_integer rhs) const
    {
        return  lhs == rhs;
    }

    bool operator() (value_integer lhs, value_bool rhs) const
    {
        return  lhs == rhs;
    }

    bool operator() (value_double lhs, value_bool rhs) const
    {
        return  lhs == rhs;
    }

    bool operator() (value_unicode_string const& lhs,
                     value_unicode_string const& rhs) const
    {
        return  (lhs == rhs) ? true: false;
    }

    template <typename T>
    bool operator() (T lhs, T rhs) const
    {
        return lhs == rhs;
    }

    template <typename T, typename U>
    bool operator() (T const& /*lhs*/, U const& /*rhs*/) const
    {
        return false;
    }
};

struct not_equals
    : public boost::static_visitor<bool>
{
    template <typename T, typename U>
    bool operator() (const T &, const U &) const
    {
        return true;
    }

    template <typename T>
    bool operator() (T lhs, T rhs) const
    {
        return lhs != rhs;
    }

    bool operator() (value_integer lhs, value_double rhs) const
    {
        return  lhs != rhs;
    }

    bool operator() (value_bool lhs, value_double rhs) const
    {
        return  lhs != rhs;
    }

    bool operator() (value_double lhs, value_integer rhs) const
    {
        return  lhs != rhs;
    }

    bool operator() (value_bool lhs, value_integer rhs) const
    {
        return  lhs != rhs;
    }

    bool operator() (value_integer lhs, value_bool rhs) const
    {
        return  lhs != rhs;
    }

    bool operator() (value_double lhs, value_bool rhs) const
    {
        return  lhs != rhs;
    }

    bool operator() (value_unicode_string const& lhs,
                     value_unicode_string const& rhs) const
    {
        return  (lhs != rhs)? true : false;
    }

    // back compatibility shim to equate empty string with null for != test
    // https://github.com/mapnik/mapnik/issues/1859
    // TODO - consider removing entire specialization at Mapnik 3.x
    bool operator() (value_null lhs, value_unicode_string const& rhs) const
    {
        boost::ignore_unused_variable_warning(lhs);
        if (rhs.isEmpty()) return false;
        return true;
    }

};

struct greater_than
    : public boost::static_visitor<bool>
{
    template <typename T, typename U>
    bool operator()(const T &, const U &) const
    {
        return false;
    }

    template <typename T>
    bool operator()(T lhs, T rhs) const
    {
        return lhs > rhs;
    }

    bool operator() (value_integer lhs, value_double rhs) const
    {
        return  lhs > rhs;
    }

    bool operator() (value_double lhs, value_integer rhs) const
    {
        return  lhs > rhs;
    }

    bool operator() (value_unicode_string const& lhs, value_unicode_string const& rhs) const
    {
        return  (lhs > rhs) ? true : false ;
    }

    bool operator() (value_null, value_null) const
    {
        return false;
    }
};

struct greater_or_equal
    : public boost::static_visitor<bool>
{
    template <typename T, typename U>
    bool operator()(const T &, const U &) const
    {
        return false;
    }

    template <typename T>
    bool operator() (T lhs, T rhs) const
    {
        return lhs >= rhs;
    }

    bool operator() (value_integer lhs, value_double rhs) const
    {
        return  lhs >= rhs;
    }

    bool operator() (value_double lhs, value_integer rhs) const
    {
        return  lhs >= rhs;
    }

    bool operator() (value_unicode_string const& lhs, value_unicode_string const& rhs) const
    {
        return ( lhs >= rhs ) ? true : false ;
    }

    bool operator() (value_null, value_null) const
    {
        return false;
    }
};

struct less_than
    : public boost::static_visitor<bool>
{
    template <typename T, typename U>
    bool operator()(const T &, const U &) const
    {
        return false;
    }

    template <typename T>
    bool operator()(T lhs, T rhs) const
    {
        return lhs < rhs;
    }

    bool operator() (value_integer lhs, value_double rhs) const
    {
        return  lhs < rhs;
    }

    bool operator() (value_double lhs, value_integer rhs) const
    {
        return  lhs < rhs;
    }

    bool operator()(value_unicode_string const& lhs,
                    value_unicode_string const& rhs ) const
    {
        return (lhs < rhs) ? true : false ;
    }

    bool operator() (value_null, value_null) const
    {
        return false;
    }
};

struct less_or_equal
    : public boost::static_visitor<bool>
{
    template <typename T, typename U>
    bool operator()(const T &, const U &) const
    {
        return false;
    }

    template <typename T>
    bool operator()(T lhs, T rhs) const
    {
        return lhs <= rhs;
    }

    bool operator() (value_integer lhs, value_double rhs) const
    {
        return  lhs <= rhs;
    }

    bool operator() (value_double lhs, value_integer rhs) const
    {
        return  lhs <= rhs;
    }

    bool operator()(value_unicode_string const& lhs,
                    value_unicode_string const& rhs ) const
    {
        return (lhs <= rhs) ? true : false ;
    }

    bool operator() (value_null, value_null) const
    {
        return false;
    }
};

template <typename V>
struct add : public boost::static_visitor<V>
{
    typedef V value_type;
    value_type operator() (value_unicode_string const& lhs ,
                           value_unicode_string const& rhs ) const
    {
        return lhs + rhs;
    }

    value_type operator() (value_double lhs, value_integer rhs) const
    {
        return lhs + rhs;
    }

    value_type operator() (value_integer lhs, value_double rhs) const
    {
        return lhs + rhs;
    }

    value_type operator() (value_unicode_string const& lhs, value_null rhs) const
    {
        boost::ignore_unused_variable_warning(rhs);
        return lhs;
    }

    value_type operator() (value_null lhs, value_unicode_string const& rhs) const
    {
        boost::ignore_unused_variable_warning(lhs);
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

    template <typename L>
    value_type operator() (L const& lhs , value_unicode_string const& rhs) const
    {
        std::string val;
        if (util::to_string(val,lhs))
            return value_unicode_string(val.c_str()) + rhs;
        return rhs;
    }

    template <typename T>
    value_type operator() (T lhs, T rhs) const
    {
        return lhs + rhs ;
    }

    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs, T2 const&) const
    {
        return lhs;
    }

    value_type operator() (value_bool lhs, value_bool rhs) const
    {
        return value_integer(lhs + rhs);
    }
};

template <typename V>
struct sub : public boost::static_visitor<V>
{
    typedef V value_type;
    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs, T2 const&) const
    {
        return lhs;
    }

    template <typename T>
    value_type operator() (T lhs, T rhs) const
    {
        return lhs - rhs ;
    }

    value_type operator() (value_unicode_string const& lhs,
                           value_unicode_string const& ) const
    {
        return lhs;
    }

    value_type operator() (value_double lhs, value_integer rhs) const
    {
        return lhs - rhs;
    }

    value_type operator() (value_integer lhs, value_double rhs) const
    {
        return lhs - rhs;
    }

    value_type operator() (value_bool lhs, value_bool rhs) const
    {
        return value_integer(lhs - rhs);
    }
};

template <typename V>
struct mult : public boost::static_visitor<V>
{
    typedef V value_type;
    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs , T2 const& ) const
    {
        return lhs;
    }
    template <typename T>
    value_type operator() (T lhs, T rhs) const
    {
        return lhs * rhs;
    }

    value_type operator() (value_unicode_string const& lhs,
                           value_unicode_string const& ) const
    {
        return lhs;
    }

    value_type operator() (value_double lhs, value_integer rhs) const
    {
        return lhs * rhs;
    }

    value_type operator() (value_integer lhs, value_double rhs) const
    {
        return lhs * rhs;
    }

    value_type operator() (value_bool lhs, value_bool rhs) const
    {
        boost::ignore_unused_variable_warning(lhs);
        boost::ignore_unused_variable_warning(rhs);
        return value_integer(0);
    }
};

template <typename V>
struct div: public boost::static_visitor<V>
{
    typedef V value_type;
    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs, T2 const&) const
    {
        return lhs;
    }

    template <typename T>
    value_type operator() (T lhs, T rhs) const
    {
        return lhs / rhs;
    }

    value_type operator() (value_bool lhs, value_bool rhs ) const
    {
        boost::ignore_unused_variable_warning(lhs);
        boost::ignore_unused_variable_warning(rhs);
        return false;
    }

    value_type operator() (value_unicode_string const& lhs,
                           value_unicode_string const&) const
    {
        return lhs;
    }

    value_type operator() (value_double lhs, value_integer rhs) const
    {
        return lhs / rhs;
    }

    value_type operator() (value_integer lhs, value_double rhs) const
    {
        return lhs / rhs;
    }
};

template <typename V>
struct mod: public boost::static_visitor<V>
{
    typedef V value_type;
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

    value_type operator() (value_unicode_string const& lhs,
                           value_unicode_string const&) const
    {
        return lhs;
    }

    value_type operator() (value_bool lhs,
                           value_bool rhs) const
    {
        boost::ignore_unused_variable_warning(lhs);
        boost::ignore_unused_variable_warning(rhs);
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
struct negate : public boost::static_visitor<V>
{
    typedef V value_type;

    template <typename T>
    value_type operator() (T val) const
    {
        return -val;
    }

    value_type operator() (value_null const& val) const
    {
        return val;
    }

    value_type operator() (value_bool val) const
    {
        return val ? value_integer(-1) : value_integer(0);
    }

    value_type operator() (value_unicode_string const& ustr) const
    {
        value_unicode_string inplace(ustr);
        return inplace.reverse();
    }
};

// converters
template <typename T>
struct convert {};

template <>
struct convert<value_bool> : public boost::static_visitor<value_bool>
{
    value_bool operator() (value_bool val) const
    {
        return val;
    }

    value_bool operator() (value_unicode_string const& ustr) const
    {
        return !ustr.isEmpty();
    }

    value_bool operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return false;
    }

    template <typename T>
    value_bool operator() (T val) const
    {
        return val > 0 ? true : false;
    }
};

template <>
struct convert<value_double> : public boost::static_visitor<value_double>
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
        to_utf8(val,utf8);
        return operator()(utf8);
    }

    value_double operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return 0.0;
    }
};

template <>
struct convert<value_integer> : public boost::static_visitor<value_integer>
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
        to_utf8(val,utf8);
        return operator()(utf8);
    }

    value_integer operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return value_integer(0);
    }
};

template <>
struct convert<std::string> : public boost::static_visitor<std::string>
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
        to_utf8(val,utf8);
        return utf8;
    }

    std::string operator() (value_double val) const
    {
        std::string str;
        util::to_string(str, val); // TODO set precision(16)
        return str;
    }

    std::string operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return "";
    }
};

struct to_unicode : public boost::static_visitor<value_unicode_string>
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

    value_unicode_string operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return value_unicode_string("");
    }
};

struct to_expression_string : public boost::static_visitor<std::string>
{
    std::string operator() (value_unicode_string const& val) const
    {
        std::string utf8;
        to_utf8(val,utf8);
        return "'" + utf8 + "'";
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
        return val ? "true":"false";
    }

    std::string operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return "null";
    }
};

} // namespace impl

namespace value_adl_barrier {

class value
{
    value_base base_;
    friend const value operator+(value const&,value const&);
    friend const value operator-(value const&,value const&);
    friend const value operator*(value const&,value const&);
    friend const value operator/(value const&,value const&);
    friend const value operator%(value const&,value const&);

public:
    value () noexcept //-- comment out for VC++11
        : base_(value_null()) {}

    value(value_integer val)
        : base_(val) {}

    value(value_double val)
        : base_(val) {}

    value(value_bool val)
        : base_(val) {}

    value(value_null val)
        : base_(val) {}

    value(value_unicode_string const& val)
        : base_(val) {}

    value (value const& other)
        : base_(other.base_) {}

    value & operator=( value const& other)
    {
        if (this == &other)
            return *this;
        base_ = other.base_;
        return *this;
    }

    value( value && other) noexcept
        :  base_(std::move(other.base_)) {}

    bool operator==(value const& other) const
    {
        return boost::apply_visitor(impl::equals(),base_,other.base_);
    }

    bool operator!=(value const& other) const
    {
        return boost::apply_visitor(impl::not_equals(),base_,other.base_);
    }

    bool operator>(value const& other) const
    {
        return boost::apply_visitor(impl::greater_than(),base_,other.base_);
    }

    bool operator>=(value const& other) const
    {
        return boost::apply_visitor(impl::greater_or_equal(),base_,other.base_);
    }

    bool operator<(value const& other) const
    {
        return boost::apply_visitor(impl::less_than(),base_,other.base_);
    }

    bool operator<=(value const& other) const
    {
        return boost::apply_visitor(impl::less_or_equal(),base_,other.base_);
    }

    value operator- () const
    {
        return boost::apply_visitor(impl::negate<value>(), base_);
    }

    value_base const& base() const
    {
        return base_;
    }

    bool is_null() const;

    template <typename T>
    T convert() const
    {
        return boost::apply_visitor(impl::convert<T>(),base_);
    }

    value_bool to_bool() const
    {
        return boost::apply_visitor(impl::convert<value_bool>(),base_);
    }

    std::string to_expression_string() const
    {
        return boost::apply_visitor(impl::to_expression_string(),base_);
    }

    std::string to_string() const
    {
        return boost::apply_visitor(impl::convert<std::string>(),base_);
    }

    value_unicode_string to_unicode() const
    {
        return boost::apply_visitor(impl::to_unicode(),base_);
    }

    value_double to_double() const
    {
        return boost::apply_visitor(impl::convert<value_double>(),base_);
    }

    value_integer to_int() const
    {
        return boost::apply_visitor(impl::convert<value_integer>(),base_);
    }
};

inline const value operator+(value const& p1,value const& p2)
{

    return value(boost::apply_visitor(impl::add<value>(),p1.base_, p2.base_));
}

inline const value operator-(value const& p1,value const& p2)
{

    return value(boost::apply_visitor(impl::sub<value>(),p1.base_, p2.base_));
}

inline const value operator*(value const& p1,value const& p2)
{

    return value(boost::apply_visitor(impl::mult<value>(),p1.base_, p2.base_));
}

inline const value operator/(value const& p1,value const& p2)
{

    return value(boost::apply_visitor(impl::div<value>(),p1.base_, p2.base_));
}

inline const value operator%(value const& p1,value const& p2)
{

    return value(boost::apply_visitor(impl::mod<value>(),p1.base_, p2.base_));
}

template <typename charT, typename traits>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             value const& v)
{
    out << v.to_string();
    return out;
}

inline std::size_t hash_value(const value& val) {
    return hash_value(val.base());
}

} // namespace value_adl_barrier

using value_adl_barrier::value;
using value_adl_barrier::operator<<;

namespace impl {

struct is_null : public boost::static_visitor<bool>
{
    bool operator() (value const& val) const
    {
        return val.is_null();
    }

    bool operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return true;
    }

    template <typename T>
    bool operator() (T const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return false;
    }

    template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
    bool operator() (boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& val)
        const
    {
        return boost::apply_visitor(*this, val);
    }
};

} // namespace impl

// constant visitor instance substitutes overloaded function
impl::is_null const is_null = impl::is_null();

inline bool value::is_null() const
{
    return boost::apply_visitor(impl::is_null(), base_);
}

} // namespace mapnik

#endif // MAPNIK_VALUE_HPP
