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

// mapnik
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/wkt/wkt_grammar_x3.hpp>

namespace mapnik {

bool from_wkt(std::string const& wkt, mapnik::geometry::geometry<double>& geom)
{
    using namespace boost::spirit;
    x3::ascii::space_type space;
    std::string::const_iterator itr = wkt.begin();
    std::string::const_iterator end = wkt.end();
    bool result;
    try
    {
        result = x3::phrase_parse(itr, end, grammar::wkt, space, geom);
    } catch (x3::expectation_failure<std::string::const_iterator> const& ex)
    {
        return false;
    }
    return result && itr == end;
}

} // namespace mapnik
