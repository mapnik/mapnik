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

#ifndef MAPNIK_UTIL_DASHARRAY_PARSER_HPP
#define MAPNIK_UTIL_DASHARRAY_PARSER_HPP

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

namespace mapnik { namespace util {

template <typename Iterator>
bool parse_dasharray(Iterator first, Iterator last, std::vector<double>& dasharray)
{
    using qi::double_;
    using qi::phrase_parse;
    using qi::_1;
    using qi::lit;
    using qi::char_;
#if BOOST_VERSION > 104200
    using qi::no_skip;
#else
    using qi::lexeme;
#endif
    using phoenix::push_back;
    // SVG 
    // dasharray ::= (length | percentage) (comma-wsp dasharray)?
    // no support for 'percentage' as viewport is unknown at load_map
    // 
    bool r = phrase_parse(first, last,
                          (double_[push_back(phoenix::ref(dasharray), _1)] %
#if BOOST_VERSION > 104200
                          no_skip[char_(", ")]
#else
                          lexeme[char_(", ")]
#endif
                          | lit("none")),
                          qi::ascii::space);
    
    if (first != last) 
        return false;
    
    return r;
}

}}

#endif // MAPNIK_UTIL_DASHARRAY_PARSER_HPP
