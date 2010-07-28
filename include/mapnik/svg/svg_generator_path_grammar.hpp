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

#ifndef SVG_GENERATOR_PATH_GRAMMAR_HPP
#define SVG_GENERATOR_PATH_GRAMMAR_HPP

// mapnik
#include <mapnik/vertex.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/ctrans.hpp>

// boost
#include <boost/fusion/include/adapt_class.hpp>
#include <boost/spirit/include/karma.hpp>

BOOST_FUSION_ADAPT_CLASS(
    mapnik::vertex_vector2<mapnik::vertex2d>::vertex_type,
    (unsigned, unsigned, obj.get<2>(), /**/)
    (mapnik::vertex_vector2<mapnik::vertex2d>::value_type, mapnik::vertex_vector2<mapnik::vertex2d>::value_type, obj.get<0>(), /**/)
    (mapnik::vertex_vector2<mapnik::vertex2d>::value_type, mapnik::vertex_vector2<mapnik::vertex2d>::value_type, obj.get<1>(), /**/)
)

namespace boost { namespace spirit { namespace traits {

    template <>
    struct is_container<mapnik::geometry2d const>
	: mpl::true_
    {};

    template <>
    struct container_iterator<mapnik::geometry2d const>
    {
	typedef mapnik::geometry2d::iterator type;
    };

    template <>
    struct begin_container<mapnik::geometry2d const>
    {
	static mapnik::geometry2d::iterator 
	call(mapnik::geometry2d const& g)
	{
		return g.begin();
	}
    };

    template <>
    struct end_container<mapnik::geometry2d const>
    {
	static mapnik::geometry2d::iterator 
	call(mapnik::geometry2d const& g)
	{
		return g.end();
	}
    };
}}}

namespace mapnik { namespace svg {

    using namespace boost::spirit;

    template <typename OutputIterator, typename PathType>
    struct svg_generator_path_grammar : karma::grammar<OutputIterator, mapnik::geometry2d()>
    {
	explicit svg_generator_path_grammar(PathType const& path_type)
	    : svg_generator_path_grammar::base_type(svg_path),
	      path_type_(path_type)
	{
	    svg_path = *(karma::int_ << karma::lit(' ') <<  karma::double_ << karma::lit(' ') << karma::double_ << karma::eol);
	}

	karma::rule<OutputIterator, mapnik::geometry2d()> svg_path;

	PathType const& path_type_;
    };
}}

#endif // SVG_GENERATOR_PATH_GRAMMAR_HPP
