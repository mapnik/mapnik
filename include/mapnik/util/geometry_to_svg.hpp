/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_TO_SVG_HPP
#define MAPNIK_GEOMETRY_TO_SVG_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/svg/geometry_svg_generator.hpp>

// boost
#include <boost/spirit/include/karma.hpp>

namespace mapnik { namespace util {

namespace karma = boost::spirit::karma;

inline bool to_svg(std::string & svg, mapnik::geometry_type const& geom)
{
    using sink_type = std::back_insert_iterator<std::string>;
    sink_type sink(svg);
    mapnik::vertex_adapter va(geom);
    static const svg::svg_path_generator<sink_type, mapnik::vertex_adapter> generator;
    bool result = karma::generate(sink, generator, geom);
    return result;
}

// TODO
// https://github.com/mapnik/mapnik/issues/1437
/*
bool to_svg(std::string & svg, mapnik::geometry_container const& geom)
{
    using sink_type = std::back_insert_iterator<std::string>;
    sink_type sink(svg);
    svg_multi_generator<sink_type> generator;
    bool result = karma::generate(sink, generator, geom);
    return result;
}
*/

}}


#endif // MAPNIK_GEOMETRY_TO_SVG_HPP
