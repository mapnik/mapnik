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

#ifndef SVG_OUTPUT_GRAMMARS_HPP
#define SVG_OUTPUT_GRAMMARS_HPP

// mapnik
#include <mapnik/vertex.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/svg/svg_output_attributes.hpp>

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/repository/include/karma_confix.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
#include <boost/fusion/include/adapt_class.hpp>
#include <boost/fusion/include/std_pair.hpp>

// std
#include <iostream>

/*!
 * mapnik::vertex2d is adapted as a fusion sequence in order to
 * be used directly by the svg_path_data_grammar (below).
 *
 * The resulting sequence is as follows: (cmd, x, y, x, y).
 * Notice that x and y are listed twice; this is because the current
 * grammar requires to consume them twice, once to retrieve their
 * original value (in map coordinates) and once to generate them
 * with their value in user coordinates (after conversion).
 */
BOOST_FUSION_ADAPT_CLASS(
    mapnik::vertex_vector2<mapnik::vertex2d>::vertex_type,
    (unsigned, unsigned, obj.get<2>(), /**/)
    (double, double, obj.get<0>(), /**/)
    (double, double, obj.get<1>(), /**/)
    (double, double, obj.get<0>(), /**/)
    (double, double, obj.get<1>(), /**/)
)

/*!
 * mapnik::svg::path_output_attributes is adapted as a fusion sequence
 * in order to be used directly by the svg_path_attributes_grammar (below).
 *
 * This adaptation is the primary reason why the attributes are stored in
 * this structure before being passed to the generate_path method.
 */
BOOST_FUSION_ADAPT_CLASS(
    mapnik::svg::path_output_attributes,
    (std::string, std::string, obj.fill_color(), /**/)
    (double, double, obj.fill_opacity(), /**/)
    (std::string, std::string, obj.stroke_color(), /**/)
    (double, double, obj.stroke_opacity(), /**/)
    (double, double, obj.stroke_width(), /**/)
    (std::string, std::string, obj.stroke_linecap(), /**/)
    (std::string, std::string, obj.stroke_linejoin(), /**/)
    (double, double, obj.stroke_dashoffset(), /**/)
)

/*!
 * mapnik::svg::rect_output_attributes is adapted as a fusion sequence
 * in order to be used directly by the svg_rect_attributes_grammar (below).
 */
BOOST_FUSION_ADAPT_CLASS(
    mapnik::svg::rect_output_attributes,
    (int, int, obj.x(), /**/)
    (int, int, obj.y(), /**/)
    (unsigned, unsigned, obj.width(), /**/)
    (unsigned, unsigned, obj.height(), /**/)
    (std::string, std::string, obj.fill_color(), /**/)
)

/*!
 * mapnik::svg::root_output_attributes is adapted as a fusion sequence
 * in order to be used directly by the svg_root_attributes_grammar (below).
 */
BOOST_FUSION_ADAPT_CLASS(
    mapnik::svg::root_output_attributes,
    (unsigned, unsigned, obj.width(), /**/)
    (unsigned, unsigned, obj.height(), /**/)
    (double, double, obj.svg_version(), /**/)
    (std::string, std::string, obj.svg_namespace_url(), /**/)
)

/*!
 * mapnik::geometry2d is adapted to conform to the concepts
 * required by Karma to be recognized as a container of
 * attributes for output generation.
 */
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

    /*!
     * @brief Structure that performs the conversion from map to user coordinates.
     * It's methods and functions are meant to be used by svg_path_data_grammar as
     * semantic actions to convert the value of vertex coordinates inside the grammar.
     *
     * It (kind of) works like a state machine, setting the value of x and y and then
     * making the conversion just after y has been set.
     */
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
    struct svg_path_data_grammar : karma::grammar<OutputIterator, mapnik::geometry2d()>
    {
	typedef path_coordinate_transformer<PathType> coordinate_transformer;
	typedef mapnik::vertex_vector2<mapnik::vertex2d>::vertex_type vertex_type;
	typedef mapnik::vertex_vector2<mapnik::vertex2d>::value_type vertex_component_type;

	explicit svg_path_data_grammar(PathType const& path_type)
	    : svg_path_data_grammar::base_type(svg_path),
	      path_type_(path_type),
	      ct_(path_type)
	{
	    using karma::int_;
	    using karma::double_;
	    using karma::_1;
	    using karma::_a;
	    using karma::eol;
	    using karma::omit;
	    using repository::confix;

	    svg_path = 
		lit("d=") 
		<< confix('"', '"')[
		    -(path_vertex % lit(' '))];

	    path_vertex = 
		path_vertex_command
		<< omit[path_vertex_component_x] 
		<< omit[path_vertex_component_y]		
		<< path_vertex_transformed_x
		<< lit(' ')
		<< path_vertex_transformed_y;

	    path_vertex_command = &int_(1) << lit('M') | lit('L');
	    
	    path_vertex_component_x = double_[_1 = _a][bind(&coordinate_transformer::set_current_x, &ct_, _a)][_a = _val];

	    path_vertex_component_y = double_[_1 = _a][bind(&coordinate_transformer::set_current_y, &ct_, _a)][_a = _val];

	    path_vertex_transformed_x = double_[_1 = _a][bind(&coordinate_transformer::current_x, &ct_, _a)];

	    path_vertex_transformed_y = double_[_1 = _a][bind(&coordinate_transformer::current_y, &ct_, _a)];
	}

	karma::rule<OutputIterator, mapnik::geometry2d()> svg_path;
	karma::rule<OutputIterator, vertex_type()> path_vertex;
	karma::rule<OutputIterator, int()> path_vertex_command;
	karma::rule<OutputIterator, vertex_component_type(), karma::locals<double> > path_vertex_component_x, path_vertex_component_y;
	karma::rule<OutputIterator, vertex_component_type(), karma::locals<double> > path_vertex_transformed_x, path_vertex_transformed_y;

	PathType const& path_type_;
	coordinate_transformer ct_;
    };

    template <typename OutputIterator>
    struct svg_path_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::path_output_attributes()>
    {
	explicit svg_path_attributes_grammar()
	    : svg_path_attributes_grammar::base_type(svg_path_attributes)
	{
	    using karma::double_;
	    using karma::string;
	    using repository::confix;

	    svg_path_attributes = 
		lit("fill=") << confix('"', '"')[string]
		<< lit(" fill-opacity=") << confix('"', '"')[double_]
		<< lit(" stroke=") << confix('"', '"')[string]
		<< lit(" stroke-opacity=") << confix('"', '"')[double_]
		<< lit(" stroke-width=") << confix('"', '"')[double_ << lit("px")]
		<< lit(" stroke-linecap=") << confix('"', '"')[string]
		<< lit(" stroke-linejoin=") << confix('"', '"')[string]
   	        << lit(" stroke-dashoffset=") << confix('"', '"')[double_ << lit("px")];
	}

	karma::rule<OutputIterator, mapnik::svg::path_output_attributes()> svg_path_attributes;
    };

    template <typename OutputIterator>
    struct svg_path_dash_array_grammar : karma::grammar<OutputIterator, mapnik::dash_array()>
    {
	explicit svg_path_dash_array_grammar()
	    : svg_path_dash_array_grammar::base_type(svg_path_dash_array)
	{
	    using karma::double_;
	    using repository::confix;

	    svg_path_dash_array = 
		lit("stroke-dasharray=") 
		<< confix('"', '"')[
		-((double_ << lit(',') << double_) % lit(','))];
	}

	karma::rule<OutputIterator, mapnik::dash_array()> svg_path_dash_array;
    };

    template <typename OutputIterator>
    struct svg_rect_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::rect_output_attributes()>
    {
	explicit svg_rect_attributes_grammar()
	    : svg_rect_attributes_grammar::base_type(svg_rect_attributes)
	{
	    using karma::int_;
	    using karma::string;
	    using repository::confix;

	    svg_rect_attributes =		
		lit("x=") << confix('"', '"')[int_]
		<< lit(" y=") << confix('"', '"')[int_]
		<< lit(" width=") << confix('"', '"')[int_ << lit("px")]
		<< lit(" height=") << confix('"', '"')[int_ << lit("px")]
		<< lit(" fill=") << confix('"', '"')[string];
	}

	karma::rule<OutputIterator, mapnik::svg::rect_output_attributes()> svg_rect_attributes;
    };

    template <typename OutputIterator>
    struct svg_root_attributes_grammar : karma::grammar<OutputIterator, mapnik::svg::root_output_attributes()>
    {
	explicit svg_root_attributes_grammar()
	    : svg_root_attributes_grammar::base_type(svg_root_attributes)
	{
	    using karma::int_;
	    using karma::string;
	    using karma::double_;
	    using repository::confix;

	    svg_root_attributes = 
		lit("width=") << confix('"', '"')[int_ << lit("px")]
		<< lit(" height=") << confix('"', '"')[int_ << lit("px")]
		<< " version=" << confix('"', '"')[double_]
		<< " xmlns=" << confix('"', '"')[string];
	}

	karma::rule<OutputIterator, mapnik::svg::root_output_attributes()> svg_root_attributes;
    };
}}

#endif // SVG_OUTPUT_GRAMMARS_HPP
