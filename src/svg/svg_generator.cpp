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
#include <mapnik/svg/svg_generator.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/spirit/include/karma.hpp>

namespace mapnik { //namespace svg {

    using namespace boost::spirit;

    template <typename OutputIterator>
    svg_generator<OutputIterator>::svg_generator(OutputIterator& output_iterator) :
	output_iterator_(output_iterator)
    {}

    template <typename OutputIterator>
    void svg_generator<OutputIterator>::generate_root() {}

    template <typename OutputIterator>
    void svg_generator<OutputIterator>::generate_rect() {}

    template <typename OutputIterator>
    void svg_generator<OutputIterator>::generate_path(geometry2d const & geom) 
    {
	karma::generate(output_iterator_, path_grammar_, geom);
    }
}}
