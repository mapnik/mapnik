/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_UTIL_CONVERSIONS_HPP
#define MAPNIK_UTIL_CONVERSIONS_HPP

// mapnik
#include <mapnik/config.hpp>

// stl
#include <string>
#include <cmath> // log10

// boost
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/karma.hpp>

// boost
#include <boost/version.hpp>
#include <boost/math/special_functions/trunc.hpp> // trunc to avoid needing C++11

#if BOOST_VERSION >= 104500
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/karma.hpp>
#else
#include <boost/lexical_cast.hpp>
#endif

namespace mapnik { namespace util {

MAPNIK_DECL bool string2int(const char * value, int & result);
MAPNIK_DECL bool string2int(std::string const& value, int & result);

MAPNIK_DECL bool string2double(std::string const& value, double & result);
MAPNIK_DECL bool string2double(const char * value, double & result);

MAPNIK_DECL bool string2float(std::string const& value, float & result);
MAPNIK_DECL bool string2float(const char * value, float & result);

#if BOOST_VERSION >= 104500
// generic
template <typename T>
bool to_string(std::string & str, T value)
{
  namespace karma = boost::spirit::karma;
  std::back_insert_iterator<std::string> sink(str);
  return karma::generate(sink, value);
}

template <typename T>
struct double_policy : boost::spirit::karma::real_policies<T>
{
    typedef boost::spirit::karma::real_policies<T> base_type;

    static int floatfield(T n) {
      using namespace boost::spirit; // for traits

      if (traits::test_zero(n))
          return base_type::fmtflags::fixed;

      T abs_n = traits::get_absolute_value(n);
      return (abs_n >= 1e16 || abs_n < 1e-4)
        ? base_type::fmtflags::scientific : base_type::fmtflags::fixed;
    }

    static unsigned precision(T n) {
      if ( n == 0.0 ) return 0;
      using namespace boost::spirit; // for traits
      return static_cast<unsigned>(15 - boost::math::trunc(log10(traits::get_absolute_value(n))));
    }

    template <typename OutputIterator>
    static bool dot(OutputIterator& sink, T n, unsigned precision) {
      if (n == 0.0) return true; // avoid trailing zeroes
      return base_type::dot(sink, n, precision);
    }

    template <typename OutputIterator>
    static bool fraction_part (OutputIterator& sink, T n
      , unsigned precision_, unsigned precision)
    {
        // NOTE: copied from karma only to avoid trailing zeroes
        //       (maybe a bug ?)

        // allow for ADL to find the correct overload for floor and log10
        using namespace std;

        using namespace boost::spirit; // for traits
        using namespace boost::spirit::karma; // for char_inserter
        using namespace boost; // for remove_const

        if ( traits::test_zero(n) ) return true; // this part added to karma

        // The following is equivalent to:
        //    generate(sink, right_align(precision, '0')[ulong], n);
        // but it's spelled out to avoid inter-modular dependencies.

        typename remove_const<T>::type digits =
            (traits::test_zero(n) ? 0 : floor(log10(n))) + 1;
        bool r = true;
        for (/**/; r && digits < precision_; digits = digits + 1)
            r = char_inserter<>::call(sink, '0');
        if (precision && r)
            r = int_inserter<10>::call(sink, n);
        return r;
    }

    template <typename CharEncoding, typename Tag, typename OutputIterator>
    static bool exponent (OutputIterator& sink, long n)
    {
        // NOTE: copied from karma to force sign in exponent
        const bool force_sign = true;

        using namespace boost::spirit; // for traits
        using namespace boost::spirit::karma; // for char_inserter, sign_inserter

        long abs_n = traits::get_absolute_value(n);
        bool r = char_inserter<CharEncoding, Tag>::call(sink, 'e') &&
                 sign_inserter::call(sink, traits::test_zero(n)
                    , traits::test_negative(n), force_sign);

        // the C99 Standard requires at least two digits in the exponent
        if (r && abs_n < 10)
            r = char_inserter<CharEncoding, Tag>::call(sink, '0');
        return r && int_inserter<10>::call(sink, abs_n);
    }

};


// specialisation for double
template <>
inline bool to_string(std::string & str, double value)
{
    namespace karma = boost::spirit::karma;
    typedef karma::real_generator<double, double_policy<double> > double_type;
    std::back_insert_iterator<std::string> sink(str);
    return karma::generate(sink, double_type(), value);
}

#else

template <typename T>
bool to_string(std::string & str, T value)
{
    try
    {
        str = boost::lexical_cast<T>(value);
        return true;
    }
    catch (std::exception const& ex)
    {
        return false;
    }
}

#endif

}}

#endif // MAPNIK_UTIL_CONVERSIONS_HPP
