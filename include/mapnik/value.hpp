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
#include <mapnik/global.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/conversions.hpp>

// boost
#include <boost/variant.hpp>
#include <boost/scoped_array.hpp>
#include <boost/concept_check.hpp>

// stl
#include <iostream>
#include <string>
#include <cmath>

// uci
#include <unicode/unistr.h>
#include <unicode/ustring.h>


namespace mapnik  {

inline void to_utf8(UnicodeString const& input, std::string & target)
{
    if (input.length() == 0) return;

    const int BUF_SIZE = 256;
    char  buf [BUF_SIZE];
    int len;

    UErrorCode err = U_ZERO_ERROR;
    u_strToUTF8(buf, BUF_SIZE, &len, input.getBuffer(), input.length(), &err);
    if (err == U_BUFFER_OVERFLOW_ERROR || err == U_STRING_NOT_TERMINATED_WARNING )
    {
        boost::scoped_array<char> buf_ptr(new char [len+1]);
        err = U_ZERO_ERROR;
        u_strToUTF8(buf_ptr.get() , len + 1, &len, input.getBuffer(), input.length(), &err);
        target.assign(buf_ptr.get() , len);
    }
    else
    {
        target.assign(buf, len);
    }
}

struct value_null
{
    template <typename T>
    value_null operator+ (T const& other) const
    {
        boost::ignore_unused_variable_warning(other);
        return *this;
    }

    template <typename T>
    value_null operator- (T const& other) const
    {
        boost::ignore_unused_variable_warning(other);
        return *this;
    }

    template <typename T>
    value_null operator* (T const& other) const
    {
        boost::ignore_unused_variable_warning(other);
        return *this;
    }

    template <typename T>
    value_null operator/ (T const& other) const
    {
        boost::ignore_unused_variable_warning(other);
        return *this;
    }

    template <typename T>
    value_null operator% (T const& other) const
    {
        boost::ignore_unused_variable_warning(other);
        return *this;
    }
};

typedef boost::variant<value_null,bool,int,double,UnicodeString> value_base;

namespace impl {

struct equals
    : public boost::static_visitor<bool>
{
    template <typename T, typename U>
    bool operator() (const T &, const U &) const
    {
        return false;
    }

    template <typename T>
    bool operator() (T lhs, T rhs) const
    {
        return lhs == rhs;
    }

    bool operator() (int lhs, double rhs) const
    {
        return  lhs == rhs;
    }

    bool operator() (double lhs, int rhs) const
    {
        return  (lhs == rhs)? true : false ;
    }

    bool operator() (UnicodeString const& lhs,
                     UnicodeString const& rhs) const
    {
        return  (lhs == rhs) ? true: false;
    }

    bool operator() (value_null, value_null) const
    {
        // this changed from false to true - https://github.com/mapnik/mapnik/issues/794
        return true;
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

    bool operator() (int lhs, double rhs) const
    {
        return  lhs != rhs;
    }

    bool operator() (double lhs, int rhs) const
    {
        return  lhs != rhs;
    }

    bool operator() (UnicodeString const& lhs,
                     UnicodeString const& rhs) const
    {
        return  (lhs != rhs)? true : false;
    }

    bool operator() (value_null, value_null) const
    {
        // TODO - needs review - https://github.com/mapnik/mapnik/issues/794
        return false;
    }

    template <typename T>
    bool operator() (value_null, const T &) const
    {
        // TODO - needs review - https://github.com/mapnik/mapnik/issues/794
        return false;
    }

    template <typename T>
    bool operator() (const T &, value_null) const
    {
        // TODO - needs review - https://github.com/mapnik/mapnik/issues/794
        return false;
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

    bool operator() (int lhs, double rhs) const
    {
        return  lhs > rhs;
    }

    bool operator() (double lhs, int rhs) const
    {
        return  lhs > rhs;
    }

    bool operator() (UnicodeString const& lhs, UnicodeString const& rhs) const
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

    bool operator() (int lhs, double rhs) const
    {
        return  lhs >= rhs;
    }

    bool operator() (double lhs, int rhs) const
    {
        return  lhs >= rhs;
    }

    bool operator() (UnicodeString const& lhs, UnicodeString const& rhs) const
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

    bool operator() (int lhs, double rhs) const
    {
        return  lhs < rhs;
    }

    bool operator() (double lhs, int rhs) const
    {
        return  lhs < rhs;
    }

    bool operator()(UnicodeString const& lhs,
                    UnicodeString const& rhs ) const
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

    bool operator() (int lhs, double rhs) const
    {
        return  lhs <= rhs;
    }

    bool operator() (double lhs, int rhs) const
    {
        return  lhs <= rhs;
    }

    bool operator()(UnicodeString const& lhs,
                    UnicodeString const& rhs ) const
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

    template <typename T>
    value_type operator() (T lhs, T rhs) const
    {
        return lhs + rhs ;
    }

    value_type operator() (UnicodeString const& lhs ,
                           UnicodeString const& rhs ) const
    {
        return lhs + rhs;
    }

    value_type operator() (double lhs, int rhs) const
    {
        return lhs + rhs;
    }

    value_type operator() (int lhs, double rhs) const
    {
        return lhs + rhs;
    }

    value_type operator() (UnicodeString const& lhs, value_null rhs) const
    {
        boost::ignore_unused_variable_warning(rhs);
        return lhs;
    }

    value_type operator() (value_null lhs, UnicodeString const& rhs) const
    {
        boost::ignore_unused_variable_warning(lhs);
        return rhs;
    }

    template <typename R>
    value_type operator() (UnicodeString const& lhs, R const& rhs) const
    {
        std::string val;
        if (util::to_string(val,rhs))
            return lhs + UnicodeString(val.c_str());
        return lhs;
    }

    template <typename L>
    value_type operator() (L const& lhs , UnicodeString const& rhs) const
    {
        std::string val;
        if (util::to_string(val,lhs))
            return UnicodeString(val.c_str()) + rhs;
        return rhs;
    }

    template <typename T1, typename T2>
    value_type operator() (T1 const& lhs, T2 const&) const
    {
        return lhs;
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
    value_type operator() (T  lhs, T rhs) const
    {
        return lhs - rhs ;
    }

    value_type operator() (UnicodeString const& lhs,
                           UnicodeString const& ) const
    {
        return lhs;
    }

    value_type operator() (double lhs, int rhs) const
    {
        return lhs - rhs;
    }

    value_type operator() (int lhs, double rhs) const
    {
        return lhs - rhs;
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

    value_type operator() (UnicodeString const& lhs,
                           UnicodeString const& ) const
    {
        return lhs;
    }

    value_type operator() (double lhs, int rhs) const
    {
        return lhs * rhs;
    }

    value_type operator() (int lhs, double rhs) const
    {
        return lhs * rhs;
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

    value_type operator() (bool lhs, bool rhs ) const
    {
        boost::ignore_unused_variable_warning(lhs);
        boost::ignore_unused_variable_warning(rhs);
        return false;
    }

    value_type operator() (UnicodeString const& lhs,
                           UnicodeString const&) const
    {
        return lhs;
    }

    value_type operator() (double lhs, int rhs) const
    {
        return lhs / rhs;
    }

    value_type operator() (int lhs, double rhs) const
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

    value_type operator() (UnicodeString const& lhs,
                           UnicodeString const&) const
    {
        return lhs;
    }

    value_type operator() (bool lhs,
                           bool rhs) const
    {
        boost::ignore_unused_variable_warning(lhs);
        boost::ignore_unused_variable_warning(rhs);
        return false;
    }

    value_type operator() (double lhs, int rhs) const
    {
        return std::fmod(lhs, rhs);
    }

    value_type operator() (int lhs, double rhs) const
    {
        return std::fmod(lhs, rhs);
    }

    value_type operator() (double lhs, double rhs) const
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

    value_type operator() (bool val) const
    {
        return val ? -1 : 0;
    }

    value_type operator() (UnicodeString const& ustr) const
    {
        UnicodeString inplace(ustr);
        return inplace.reverse();
    }
};

struct to_bool : public boost::static_visitor<bool>
{
    bool operator() (bool val) const
    {
        return val;
    }

    bool operator() (UnicodeString const& ustr) const
    {
        boost::ignore_unused_variable_warning(ustr);
        return true;
    }

    bool operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return false;
    }

    template <typename T>
    bool operator() (T val) const
    {
        return val > 0 ? true : false;
    }
};

struct to_string : public boost::static_visitor<std::string>
{
    template <typename T>
    std::string operator() (T val) const
    {
        std::string str;
        util::to_string(str, val);
        return str;
    }

    // specializations
    std::string operator() (UnicodeString const& val) const
    {
        std::string utf8;
        to_utf8(val,utf8);
        return utf8;
    }

    std::string operator() (double val) const
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

struct to_unicode : public boost::static_visitor<UnicodeString>
{

    template <typename T>
    UnicodeString operator() (T val) const
    {
        std::string str;
        util::to_string(str,val);
        return UnicodeString(str.c_str());
    }

    // specializations
    UnicodeString const& operator() (UnicodeString const& val) const
    {
        return val;
    }

    UnicodeString operator() (double val) const
    {
        std::string str;
        util::to_string(str,val);
        return UnicodeString(str.c_str());
    }

    UnicodeString operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return UnicodeString("");
    }
};

struct to_expression_string : public boost::static_visitor<std::string>
{
    std::string operator() (UnicodeString const& val) const
    {
        std::string utf8;
        to_utf8(val,utf8);
        return "'" + utf8 + "'";
    }

    std::string operator() (double val) const
    {
        std::string output;
        util::to_string(output,val); // TODO precision(16)
        return output;
    }

    std::string operator() (bool val) const
    {
        return val ? "true":"false";
    }

    std::string operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return "null";
    }

    template <typename T>
    std::string operator() (T val) const
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
};

struct to_double : public boost::static_visitor<double>
{
    double operator() (int val) const
    {
        return static_cast<double>(val);
    }

    double operator() (double val) const
    {
        return val;
    }

    double operator() (std::string const& val) const
    {
        double result;
        if (util::string2double(val,result))
            return result;
        return 0;
    }
    double operator() (UnicodeString const& val) const
    {
        std::string utf8;
        to_utf8(val,utf8);
        return operator()(utf8);
    }

    double operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return 0.0;
    }
};

struct to_int : public boost::static_visitor<double>
{
    int operator() (int val) const
    {
        return val;
    }

    int operator() (double val) const
    {
        return rint(val);
    }

    int operator() (std::string const& val) const
    {
        int result;
        if (util::string2int(val,result))
            return result;
        return 0;
    }
    int operator() (UnicodeString const& val) const
    {
        std::string utf8;
        to_utf8(val,utf8);
        return operator()(utf8);
    }

    int operator() (value_null const& val) const
    {
        boost::ignore_unused_variable_warning(val);
        return 0;
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
    value ()
        : base_(value_null()) {}

    template <typename T> value(T _val_)
        : base_(_val_) {}

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

    bool to_bool() const
    {
        return boost::apply_visitor(impl::to_bool(),base_);
    }

    std::string to_expression_string() const
    {
        return boost::apply_visitor(impl::to_expression_string(),base_);
    }

    std::string to_string() const
    {
        return boost::apply_visitor(impl::to_string(),base_);
    }

    UnicodeString to_unicode() const
    {
        return boost::apply_visitor(impl::to_unicode(),base_);
    }

    double to_double() const
    {
        return boost::apply_visitor(impl::to_double(),base_);
    }

    double to_int() const
    {
        return boost::apply_visitor(impl::to_int(),base_);
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
