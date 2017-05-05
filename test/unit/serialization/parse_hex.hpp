/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_PARSE_HEX_HPP
#define MAPNIK_PARSE_HEX_HPP

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace util {

template <typename Out>
bool parse_hex(std::string const& input, Out & output)
{
    using boost::spirit::x3::lit;
    auto itr = input.begin();
    auto end = input.end();
    using hex2 = boost::spirit::x3::uint_parser< unsigned, 16, 2, 2 >;
    return boost::spirit::x3::parse(itr, end, -(lit("\\x") | lit("0x")) > *hex2(), output);
}

}}


#endif // MAPNIK_PARSE_HEX_HPP
