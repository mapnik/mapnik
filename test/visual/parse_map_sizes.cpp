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

#include "parse_map_sizes.hpp"

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/deque.hpp>
#pragma GCC diagnostic pop

namespace visual_tests {

namespace x3 = boost::spirit::x3;
namespace fusion = boost::fusion;

using x3::attr;
using x3::ulong_;

using map_dimspec = fusion::deque<std::size_t, std::size_t, std::size_t>;

auto const map_dimspec_rule = x3::rule<class map_dimspec_rule, map_dimspec>{}
    = ulong_ >> '+' >> ulong_ >> '+' >> ulong_
    | attr(0UL)     >> ulong_        >> attr(0UL)
    ;

auto const map_size_rule = x3::rule<class map_size_rule, map_size> {}
    = map_dimspec_rule[(
            [](auto & ctx) {
                auto & v = _val(ctx);
                auto a1 = fusion::at_c<1>(_attr(ctx));
                v.margin_left = fusion::at_c<0>(_attr(ctx));
                v.margin_right = fusion::at_c<2>(_attr(ctx));
                v.width = v.margin_left + a1 + v.margin_right;
            })]
    >> ','
    >> map_dimspec_rule[(
            [](auto & ctx) {
                auto & v = _val(ctx);
                auto a1 = fusion::at_c<1>(_attr(ctx));
                v.margin_top = fusion::at_c<0>(_attr(ctx));
                v.margin_bottom = fusion::at_c<2>(_attr(ctx));
                v.height = v.margin_top + a1 + v.margin_bottom;
            })]
    ;
auto const map_sizes_grammar = x3::rule<class map_sizes_grammar_type, std::vector<map_size> > {} =
    map_size_rule  % ';' ;

void parse_map_sizes(std::string const & str, std::vector<map_size> & sizes)
{
    boost::spirit::x3::ascii::space_type space;
    std::string::const_iterator iter = str.begin();
    std::string::const_iterator end = str.end();
    if (!boost::spirit::x3::phrase_parse(iter, end, map_sizes_grammar, space, sizes))
    {
        throw std::runtime_error("Failed to parse list of sizes: '" + str + "'");
    }
}

}
