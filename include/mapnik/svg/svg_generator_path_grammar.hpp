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
#include <boost/spirit/include/karma.hpp>
#include <boost/fusion/include/adapt_class.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>

BOOST_FUSION_ADAPT_CLASS(
    mapnik::vertex_vector2<mapnik::vertex2d>::vertex_type,
    (unsigned, unsigned, obj.get<2>(), /**/)
    (mapnik::vertex_vector2<mapnik::vertex2d>::value_type, mapnik::vertex_vector2<mapnik::vertex2d>::value_type, obj.get<0>(), /**/)
    (mapnik::vertex_vector2<mapnik::vertex2d>::value_type, mapnik::vertex_vector2<mapnik::vertex2d>::value_type, obj.get<1>(), /**/)
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
    using namespace boost::phoenix;

    template <typename PathType>
    struct path_coordinate_transformer
    {
	explicit path_coordinate_transformer(PathType const& path_type)
	    : path_type_(path_type),
	      current_x_(0.0),
	      current_y_(0.0)
	{}

	void set_current_x(double& x)
	{
	    current_x_ = x;
	}

	void set_current_y(double& y)
	{
	    current_y_ = y;
	    path_type_.vertex(&current_x_, &current_y_);
	}

	void current_x(double& x) const
	{
	    x = current_x_;
	}

	void current_y(double& y) const
	{
	    y = current_y_;
	}

	PathType const& path_type_;
	double current_x_;
	double current_y_;	
    };

    template <typename OutputIterator, typename PathType>
    struct svg_generator_path_grammar : karma::grammar<OutputIterator, mapnik::geometry2d()>
    {
	typedef path_coordinate_transformer<PathType> coordinate_transformer;
	typedef mapnik::vertex_vector2<mapnik::vertex2d>::vertex_type vertex_type;
	typedef mapnik::vertex_vector2<mapnik::vertex2d>::value_type vertex_component_type;

	explicit svg_generator_path_grammar(PathType const& path_type)
	    : svg_generator_path_grammar::base_type(svg_path),
	      path_type_(path_type),
	      ct_(path_type)
	{
	    using karma::int_;
	    using karma::double_;
	    using karma::eol;
	    using karma::omit;
	    using karma::_1;
	    using karma::_a;

	    svg_path = *(path_vertex);
	    path_vertex = int_ 
		<< omit[path_vertex_component_x] << omit[path_vertex_component_y]
		<< path_vertex_transformed_x
		<< ' '
		<< path_vertex_transformed_y
		<< eol;
	    path_vertex_component_x = double_[_1 = _a][bind(&coordinate_transformer::set_current_x, &ct_, _a)][_a = _val];
	    path_vertex_component_y = double_[_1 = _a][bind(&coordinate_transformer::set_current_y, &ct_, _a)][_a = _val];
	    path_vertex_transformed_x = double_[_1 = _a][bind(&coordinate_transformer::current_x, &ct_, _a)];
	    path_vertex_transformed_y = double_[_1 = _a][bind(&coordinate_transformer::current_y, &ct_, _a)];
	}

	karma::rule<OutputIterator, mapnik::geometry2d()> svg_path;
	karma::rule<OutputIterator, vertex_type()> path_vertex;
	karma::rule<OutputIterator, vertex_component_type(), karma::locals<double> > path_vertex_component_x, path_vertex_component_y;
	karma::rule<OutputIterator, vertex_component_type(), karma::locals<double> > path_vertex_transformed_x, path_vertex_transformed_y;

	PathType const& path_type_;
	coordinate_transformer ct_;
    };
}}

#endif // SVG_GENERATOR_PATH_GRAMMAR_HPP
