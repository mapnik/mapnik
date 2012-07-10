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
// boost
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/karma.hpp>

// boost
#include <boost/version.hpp>

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
    static int floatfield(T n) { return base_type::fmtflags::fixed; }
    static unsigned precision(T n) { return 16 ;}
};


// specialisation for double
template <>
inline bool to_string(std::string & str, double value)
{
    namespace karma = boost::spirit::karma;
    typedef boost::spirit::karma::real_generator<double, double_policy<double> > double_type;
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
