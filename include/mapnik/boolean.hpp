/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#ifndef MAPNIK_BOOLEAN_HPP
#define MAPNIK_BOOLEAN_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/util/conversions.hpp>

// std
#include <iosfwd>
#include <string>

namespace mapnik
{

class MAPNIK_DECL boolean_type
{
public:
    boolean_type()
        : b_(false)  {}
    boolean_type(bool b)
        : b_(b) {}
    boolean_type(boolean_type const& b)
        : b_(b.b_) {}

    operator bool() const
    {
        return b_;
    }

    boolean_type & operator =(boolean_type const& other)
    {
        if (this == &other)
            return *this;
        b_ = other.b_;
        return *this;
    }

private:
    bool b_;
};

// Special stream input operator for boolean_type values
template <typename charT, typename traits>
std::basic_istream<charT, traits> &
operator >> ( std::basic_istream<charT, traits> & s, boolean_type & b )
{
    if ( s )
    {
        std::string word;
        s >> word;
        bool result;
        if (util::string2bool(word,result)) b = result;
    }
    return s;
}

template <typename charT, typename traits>
std::basic_ostream<charT, traits> &
operator << ( std::basic_ostream<charT, traits> & s, boolean_type const& b )
{
    s << ( b ? "true" : "false" );
    return s;
}

}

#endif // MAPNIK_BOOLEAN_HPP
