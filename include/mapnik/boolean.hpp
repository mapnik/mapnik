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
#ifndef MAPNIK_BOOLEAN_HPP
#define MAPNIK_BOOLEAN_HPP

// std
#include <istream>

// boost
#include <boost/algorithm/string.hpp>

namespace mapnik
{
/** Helper for class bool */
class boolean {
public:
    boolean(): b_(false)  {}
    boolean(bool b) : b_(b) {}
    boolean(boolean const& b) : b_(b.b_) {}

    operator bool() const
    {
        return b_;
    }

    boolean & operator = (boolean const& other)
    {
        b_ = other.b_;
        return * this;
    }

    boolean & operator = (bool other)
    {
        b_ = other;
        return * this;
    }

private:
    bool b_;
};

/** Special stream input operator for boolean values */
template <typename charT, typename traits>
std::basic_istream<charT, traits> &
operator >> ( std::basic_istream<charT, traits> & s, boolean & b )
{
    std::string word;
    s >> word;
    boost::algorithm::to_lower(word);
    if ( s )
    {
        if ( word == "true" || word == "yes" || word == "on" ||
             word == "1")
        {
            b = true;
        }
        else if ( word == "false" || word == "no" || word == "off" ||
                  word == "0")
        {
            b = false;
        }
        else
        {
            s.setstate( std::ios::failbit );
        }
    }
    return s;
}

template <typename charT, typename traits>
std::basic_ostream<charT, traits> &
operator << ( std::basic_ostream<charT, traits> & s, boolean const& b )
{
    s << ( b ? "true" : "false" );
    return s;
}

}

#endif // MAPNIK_BOOLEAN_HPP
