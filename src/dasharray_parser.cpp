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

// mapnik
#include <mapnik/util/dasharray_parser.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

namespace util {

namespace {
inline bool setup_dashes(std::vector<double> & buf, dash_array & dash)
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
}


bool parse_dasharray(std::string const& value, dash_array & dash)
{
    using namespace boost::spirit;
    using x3::no_skip;
    using x3::double_;
    using x3::char_;
    boost::spirit::x3::ascii::space_type space;
    // SVG
    // dasharray ::= (length | percentage) (comma-wsp dasharray)?
    // no support for 'percentage' as viewport is unknown at load_map
    //
    std::vector<double> buf;
    auto first = value.begin();
    auto last = value.end();
    bool r = x3::phrase_parse(first, last,
                              (double_ % no_skip[char_(", ")] | "none"),
                              space, buf);
    if (r &&  first == last)
    {
        return setup_dashes(buf, dash);
    }
    return false;
}

} // end namespace util

} // end namespace mapnik
