/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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
#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/svg/svg_points_grammar.hpp>
// agg
#include <mapnik/svg/agg_svg_path_renderer.h>
// stl
#include <string>

namespace mapnik { namespace svg { 

template <typename PathType>
bool parse_points(std::string const& wkt, PathType & p)
{
    using namespace boost::spirit;
    typedef std::string::const_iterator iterator_type;
    typedef ascii::space_type skip_type;
    svg_points_grammar<iterator_type,skip_type,PathType> g(p);
    iterator_type first = wkt.begin();
    iterator_type last =  wkt.end();
    return qi::phrase_parse(first, last, g, skip_type());
}

template bool parse_points<agg::svg::path_renderer>(std::string const&, agg::svg::path_renderer&);

}}
