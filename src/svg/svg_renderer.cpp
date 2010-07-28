/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

// mapnik
#include <mapnik/svg_renderer.hpp>
#include <mapnik/svg/svg_generator_path_grammar.hpp>

// stl
#ifdef MAPNIK_DEBUG
#include <iostream>
#endif
#include <ostream>

// boost
#include <boost/fusion/tuple.hpp>

// boost.spirit
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/repository/include/karma_confix.hpp>

namespace karma = boost::spirit::karma;
namespace repository = boost::spirit::repository;
namespace fusion = boost::fusion;

namespace mapnik
{
    /*
     * XML_DECLARATION and SVG_DTD comprise the XML header of the SVG document.
     * They are required for producing standard compliant XML documents.
     */
    template <typename T>
    const std::string svg_renderer<T>::XML_DECLARATION = "<?xml version=\"1.0\" standalone=\"no\"?>";
    template <typename T>
    const std::string svg_renderer<T>::SVG_DTD = "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">";	

    template <typename T>
    const double svg_renderer<T>::SVG_VERSION = 1.1;
    template <typename T>
    const std::string svg_renderer<T>::SVG_NAMESPACE_URL = "http://www.w3.org/2000/svg";

    template <typename T>
    svg_renderer<T>::svg_renderer(Map const& m, T & output_iterator, unsigned offset_x, unsigned offset_y) :
	feature_style_processor<svg_renderer>(m),
	output_iterator_(output_iterator),
	width_(m.width()),
	height_(m.height()),
	t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
	generator_(output_iterator)
    {
	// nothing yet.
    }

    template <typename T>
    svg_renderer<T>::~svg_renderer() {}

    template <typename T>
    void svg_renderer<T>::start_map_processing(Map const& map)
    {
	#ifdef MAPNIK_DEBUG
	std::clog << "start map processing" << std::endl;
	#endif

	// should I move these lines to the constructor?
	// agg_renderer processes the background color of the map in the constructor.

	using namespace karma;
	using karma::string;
	using repository::confix;
	using fusion::tuple;

	// generate XML header.
	generate(
	    output_iterator_,
	    string << eol << string << eol, 
	    XML_DECLARATION, SVG_DTD);

	// generate SVG root element opening tag.
	// the root element defines the size of the image,
	// which is taken from the map's dimensions.

	generate(
	    output_iterator_,
	    confix("<", ">")[
		"svg width=" << confix('"', '"')[int_ << string]
		<< " height=" << confix('"', '"')[int_ << string]
		<< " version=" << confix('"', '"')[float_]
		<< " xmlns=" << confix('"', '"')[string]]
	    << eol,
	    tuple<int, std::string>(width_, "px"), tuple<int, std::string>(height_, "px"), SVG_VERSION, SVG_NAMESPACE_URL);

	boost::optional<color> const& bgcolor = map.background();
	if(bgcolor)
	{
	    // generate background color as a rectangle that spans the whole image.
	    generate(
		output_iterator_,
		confix("<", "/>")[
		    "rect x=" << confix('"', '"')[int_]
		    << " y=" << confix('"', '"')[int_]
		    << " width=" << confix('"', '"')[int_ << string]
		    << " height=" << confix('"', '"')[int_ << string]
		    << " style=" << confix('"', '"')["fill: " << string]]
		<< eol,
		0, 0, tuple<int, std::string>(width_, "px"), tuple<int, std::string>(height_, "px"), bgcolor->to_hex_string());
	}
    }

    template <typename T>
    void svg_renderer<T>::end_map_processing(Map const& map)
    {
	using karma::generate;
	using karma::lit;

	// generate SVG root element closing tag.
	generate(
	    output_iterator_,
	    lit("</svg>"));

	#ifdef MAPNIK_DEBUG
	std::clog << "end map processing" << std::endl;
	#endif
    }

    template <typename T>
    void svg_renderer<T>::start_layer_processing(layer const& lay)
    {
	// nothing yet.

	#ifdef MAPNIK_DEBUG
	std::clog << "start layer processing: " << lay.name() << std::endl;
	#endif
    }
    
    template <typename T>
    void svg_renderer<T>::end_layer_processing(layer const& lay)
    {
	// nothing yet.

	#ifdef MAPNIK_DEBUG
	std::clog << "end layer processing: " << lay.name() << std::endl;
	#endif
    }

    template class svg_renderer<std::ostream_iterator<char> >;
}
