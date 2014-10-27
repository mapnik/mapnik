/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_container.hpp>
#include <mapnik/wkt/wkt_grammar.hpp>
#include <mapnik/wkt/wkt_generator_grammar.hpp>
#include <boost/spirit/include/karma.hpp>

// stl
#include <string>

namespace mapnik {

inline bool from_wkt(std::string const& wkt, mapnik::geometry_container & paths)
{
    using namespace boost::spirit;
    static const mapnik::wkt::wkt_collection_grammar<std::string::const_iterator> g;
    ascii::space_type space;
    std::string::const_iterator first = wkt.begin();
    std::string::const_iterator last =  wkt.end();
    return qi::phrase_parse(first, last, g, space, paths);
}

inline bool to_wkt(mapnik::geometry_container const& paths, std::string& wkt)
{
    using sink_type = std::back_insert_iterator<std::string>;
    static const mapnik::wkt::wkt_multi_generator<sink_type, mapnik::geometry_container> generator;
    sink_type sink(wkt);
    return boost::spirit::karma::generate(sink, generator, paths);
}


}

#endif // MAPNIK_WKT_FACTORY_HPP
