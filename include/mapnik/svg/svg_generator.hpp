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

#ifndef MAPNIK_SVG_GENERATOR_HPP
#define MAPNIK_SVG_GENERATOR_HPP

// mapnik
#include <mapnik/ctrans.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/svg/svg_generator_path_grammar.hpp>

// boost
#include <boost/utility.hpp>

namespace mapnik { namespace svg {

    template <typename OutputIterator>
    class svg_generator : private boost::noncopyable
    {
	typedef coord_transform2<CoordTransform, geometry2d> path_type;
	typedef svg::svg_generator_path_grammar<OutputIterator, path_type> path_grammar;

    public:
	explicit svg_generator(OutputIterator& output_iterator);
	~svg_generator();

	void generate_root();
	void generate_rect();
	void generate_path(path_type const& path);
	
    private:
	OutputIterator& output_iterator_;
    };
}}

#endif // MAPNIK_SVG_GENERATOR_HPP
