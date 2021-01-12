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

#ifndef MAPNIK_UTIL_CHAR_ARRAY_BUFFER_HPP
#define MAPNIK_UTIL_CHAR_ARRAY_BUFFER_HPP

#include <streambuf>

namespace mapnik { namespace util {

// ref https://artofcode.wordpress.com/2010/12/12/deriving-from-stdstreambuf/

class char_array_buffer : public std::streambuf
{
public:
    char_array_buffer(char const* data, std::size_t size)
        : begin_(data), end_(data + size), current_(data) {}

private:
    int_type underflow()
    {
        if (current_ == end_)
        {
            return traits_type::eof();
        }
        return traits_type::to_int_type(*current_);
    }

    int_type uflow()
    {
        if (current_ == end_)
        {
            return traits_type::eof();
        }
        return traits_type::to_int_type(*current_++);
    }

    int_type pbackfail(int_type ch)
    {
        if (current_ == begin_ || (ch != traits_type::eof() && ch != current_[-1]))
        {
            return traits_type::eof();
        }
        return traits_type::to_int_type(*--current_);
    }

    std::streamsize showmanyc()
    {
        return end_ - current_;
    }

    pos_type seekpos(pos_type off,
                     std::ios_base::openmode /*which*/)
    {
        current_ = std::min(begin_ + off, end_);
        return pos_type(off_type(current_ - begin_));
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                     std::ios_base::openmode which = std::ios_base::in | std::ios_base::out )
    {
        if (dir == std::ios_base::beg) current_ = std::min(begin_ + off, end_);
        else if (dir == std::ios_base::cur) current_ = std::min(current_ + off, end_);
        else current_ = std::max(end_ - off, begin_); // dir == std::ios_base::end
        return pos_type(off_type(current_ - begin_));
    }
    char const * const begin_;
    char const * const end_;
    char const * current_;
};

}}


#endif // MAPNIK_UTIL_CHAR_ARRAY_BUFFER_HPP
