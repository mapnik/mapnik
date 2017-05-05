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
#include <mapnik/geometry/geometry_types.hpp>
#include <mapnik/svg/geometry_svg_generator.hpp>

namespace mapnik { namespace svg {

    namespace karma = boost::spirit::karma;
    namespace phoenix = boost::phoenix;

    template <typename OutputIterator, typename Path>
    svg_path_generator<OutputIterator,Path>::svg_path_generator()
        : svg_path_generator::base_type(svg)
    {
        boost::spirit::karma::uint_type uint_;
        boost::spirit::karma::_val_type _val;
        boost::spirit::karma::_1_type _1;
        boost::spirit::karma::lit_type lit;

        svg = point | linestring | polygon
            ;

        point = &uint_(mapnik::geometry::geometry_types::Point)[_1 = _type(_val)]
            << svg_point [_1 = _first(_val)]
            ;

        svg_point = &uint_
            << lit("cx=\"") << coordinate
            << lit("\" cy=\"") << coordinate
            << lit('\"')
            ;

        linestring = &uint_(mapnik::geometry::geometry_types::LineString)[_1 = _type(_val)]
            << lit("d=\"") << svg_path << lit("\"")
            ;

        polygon = &uint_(mapnik::geometry::geometry_types::Polygon)[_1 = _type(_val)]
            << lit("d=\"") << svg_path << lit("\"")
            ;

        svg_path %= ((&uint_(mapnik::SEG_MOVETO) << lit('M')
                      | &uint_(mapnik::SEG_LINETO) << lit('L'))
                     << coordinate << lit(' ') << coordinate) % lit(' ')
                     ;

    }

}}
