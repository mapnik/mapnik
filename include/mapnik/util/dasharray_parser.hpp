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
#include <boost/spirit/include/phoenix_function.hpp>

#include <vector>
#include <string>

namespace mapnik { namespace util {

template <typename Iterator>
bool parse_dasharray(Iterator first, Iterator last, std::vector<double>& dasharray)
{
    using namespace boost::spirit;
    qi::double_type double_;
    qi::_1_type _1;
    qi::lit_type lit;
    qi::char_type char_;
    qi::ascii::space_type space;
    qi::no_skip_type no_skip;
    // SVG
    // dasharray ::= (length | percentage) (comma-wsp dasharray)?
    // no support for 'percentage' as viewport is unknown at load_map
    //
    bool r = qi::phrase_parse(first, last,
                          (double_[boost::phoenix::push_back(boost::phoenix::ref(dasharray), _1)] %
                          no_skip[char_(", ")]
                          | lit("none")),
                          space);
    if (first != last)
    {
        return false;
    }
    return r;
}

inline bool add_dashes(std::vector<double> & buf, std::vector<std::pair<double,double> > & dash)
{
    if (buf.empty()) return false;
    size_t size = buf.size();
    if (size % 2 == 1)
    {
        buf.insert(buf.end(),buf.begin(),buf.end());
    }
    std::vector<double>::const_iterator pos = buf.begin();
    while (pos != buf.end())
    {
        if (*pos > 0.0 || *(pos+1) > 0.0) // avoid both dash and gap eq 0.0
        {
            dash.emplace_back(*pos,*(pos + 1));
        }
        pos +=2;
    }
    return !buf.empty();
}

}}

#endif // MAPNIK_UTIL_DASHARRAY_PARSER_HPP
