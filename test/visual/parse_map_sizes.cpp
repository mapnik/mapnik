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

#include "parse_map_sizes.hpp"

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
MAPNIK_DISABLE_WARNING_POP

BOOST_FUSION_ADAPT_STRUCT(visual_tests::map_size, (std::size_t, width)(std::size_t, height))

namespace visual_tests {

namespace x3 = boost::spirit::x3;
using x3::ulong_;
auto const map_size_rule = x3::rule<class map_size_rule, map_size>{} = ulong_ >> ',' >> ulong_;
auto const map_sizes_grammar = x3::rule<class map_sizes_grammar_type, std::vector<map_size>>{} = map_size_rule % ';';

void parse_map_sizes(std::string const& str, std::vector<map_size>& sizes)
{
    boost::spirit::x3::ascii::space_type space;
    std::string::const_iterator iter = str.begin();
    std::string::const_iterator end = str.end();
    if (!boost::spirit::x3::phrase_parse(iter, end, map_sizes_grammar, space, sizes))
    {
        throw std::runtime_error("Failed to parse list of sizes: '" + str + "'");
    }
}

} // namespace visual_tests
