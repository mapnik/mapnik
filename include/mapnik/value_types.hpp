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

#ifndef MAPNIK_VALUE_TYPES_HPP
#define MAPNIK_VALUE_TYPES_HPP

// icu
#include <unicode/unistr.h>  // for UnicodeString

// boost
#include <boost/concept_check.hpp>

// stl
#include <iosfwd> // for ostream

namespace mapnik  {

#ifdef BIGINT
//typedef boost::long_long_type value_integer;
typedef long long value_integer;
#else
typedef int value_integer;
#endif

typedef double value_double;
typedef U_NAMESPACE_QUALIFIER UnicodeString value_unicode_string;
typedef bool value_bool;

struct value_null
{
    bool operator==(value_null const& other) const
    {
        boost::ignore_unused_variable_warning(other);
        return true;
    }

    template <typename T>
    bool operator==(T const& other) const
    {
        boost::ignore_unused_variable_warning(other);
        return false;
    }

    bool operator!=(value_null const& other) const
    {
        boost::ignore_unused_variable_warning(other);
        return false;
    }

    template <typename T>
    bool operator!=(T const& other) const
    {
        boost::ignore_unused_variable_warning(other);
        return true;
    }

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

inline std::size_t hash_value(const value_null& val)
{
    boost::ignore_unused_variable_warning(val);
    return 0;
}

inline std::ostream& operator<< (std::ostream & out,value_null const& v)
{
    boost::ignore_unused_variable_warning(v);
    return out;
}
inline std::istream& operator>> ( std::istream & s, value_null & null )
{
    boost::ignore_unused_variable_warning(null);
    return s;
}


} // namespace mapnik

#endif // MAPNIK_VALUE_TYPES_HPP
