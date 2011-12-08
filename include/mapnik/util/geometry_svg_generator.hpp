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

//$Id$

#ifndef MAPNIK_GEOMETRY_SVG_GENERATOR_HPP
#define MAPNIK_GEOMETRY_SVG_GENERATOR_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/util/vertex_iterator.hpp>
#include <mapnik/util/container_adapter.hpp>

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

//#define BOOST_SPIRIT_USE_PHOENIX_V3 1

namespace mapnik { namespace util {

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

template <typename OutputIterator>
bool generate_svg (OutputIterator sink, mapnik::geometry_type const& geom)
{
    using boost::spirit::karma::double_;
    using boost::spirit::karma::uint_;
    using boost::spirit::karma::generate;
    return generate (sink,
                     // begin grammar                  
                     ((&uint_(mapnik::SEG_MOVETO) << 'M' | 'L')
                      << " " << double_ << " " << double_) % ' '
                     // end grammar
                    , geom);
}

}}

#endif // MAPNIK_GEOMETRY_SVG_GENERATOR_HPP
