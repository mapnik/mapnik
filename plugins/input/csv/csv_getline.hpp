/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_CSV_GETLINE_HPP
#define MAPNIK_CSV_GETLINE_HPP

#include <string>
#include <istream>
#include <streambuf>

namespace csv_utils {

template<class CharT, class Traits, class Allocator>
std::basic_istream<CharT, Traits>&
  getline_csv(std::istream& is, std::basic_string<CharT, Traits, Allocator>& s, CharT delim, CharT quote)
{
    typename std::basic_string<CharT, Traits, Allocator>::size_type nread = 0;
    typename std::basic_istream<CharT, Traits>::sentry sentry(is, true);
    if (sentry)
    {
        std::basic_streambuf<CharT, Traits>* buf = is.rdbuf();
        s.clear();
        bool has_quote = false;
        while (nread < s.max_size())
        {
            int c1 = buf->sbumpc();
            if (Traits::eq_int_type(c1, Traits::eof()))
            {
                is.setstate(std::ios_base::eofbit);
                break;
            }
            else
            {
                ++nread;
                CharT c = Traits::to_char_type(c1);
                if (Traits::eq(c, quote))
                    has_quote = !has_quote;
                if (!Traits::eq(c, delim) || has_quote)
                    s.push_back(c);
                else
                    break; // Character is extracted but not appended.
            }
        }
    }
    if (nread == 0 || nread >= s.max_size())
        is.setstate(std::ios_base::failbit);

    return is;
}

} // namespace csv_utils

#endif // MAPNIK_CSV_GETLINE_HPP
