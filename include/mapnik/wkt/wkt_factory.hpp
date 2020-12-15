/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_WKT_FACTORY_HPP
#define MAPNIK_WKT_FACTORY_HPP

#include <boost/phoenix/phoenix.hpp>

// mapnik
#include <mapnik/geometry.hpp>

#include <mapnik/wkt/wkt_grammar.hpp>
#include <mapnik/wkt/wkt_generator_grammar.hpp>

// stl
#include <string>

namespace mapnik {

inline bool from_wkt(std::string const& wkt, mapnik::geometry::geometry<double> & geom)
{
    using namespace boost::spirit;
    static const mapnik::wkt::wkt_grammar<std::string::const_iterator> g;
    ascii::space_type space;
    std::string::const_iterator first = wkt.begin();
    std::string::const_iterator last =  wkt.end();
    return qi::phrase_parse(first, last, (g)(boost::phoenix::ref(geom)), space);
}

}

#endif // MAPNIK_WKT_FACTORY_HPP
