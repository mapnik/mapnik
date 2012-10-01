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

#ifndef MAPNIK_GEOMETRY_TO_WKT_HPP
#define MAPNIK_GEOMETRY_TO_WKT_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/util/geometry_wkt_generator.hpp>

// boost
#include <boost/spirit/include/karma.hpp>

namespace mapnik { namespace util {

namespace karma = boost::spirit::karma;

bool to_wkt(std::string & wkt, mapnik::geometry_type const& geom)
{
    typedef std::back_insert_iterator<std::string> sink_type;
    sink_type sink(wkt);
    wkt_generator<sink_type, mapnik::geometry_type> generator(true);
    bool result = karma::generate(sink, generator, geom);
    return result;
}

bool to_wkt(std::string & wkt, mapnik::geometry_container const& geom)
{
    typedef std::back_insert_iterator<std::string> sink_type;
    sink_type sink(wkt);
    wkt_multi_generator<sink_type, mapnik::geometry_container> generator;
    bool result = karma::generate(sink, generator, geom);
    return result;
}

}}


#endif // MAPNIK_GEOMETRY_TO_WKT_HPP
