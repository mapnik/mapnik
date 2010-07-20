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

// stl
#ifdef MAPNIK_DEBUG
#include <iostream>
#endif
#include <sstream>

// boost.spirit
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/repository/include/karma_confix.hpp>

namespace karma = boost::spirit::karma;
namespace repository = boost::spirit::repository;
namespace ascii = karma::ascii;

namespace mapnik
{
    /*
     * XML_DECLARATION and SVG_DTD comprise the XML header of the SVG document.
     * They are required for producing standard compliant XML documents.
     * They are stored in svg_renderer, but they might move to somewhere else.
     */
    template <typename T>
    const std::string svg_renderer<T>::XML_DECLARATION = "<?xml version=\"1.0\" standalone=\"no\"?>";
    template <typename T>
    const std::string svg_renderer<T>::SVG_DTD = "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">";	

    /*
     * I'm not sure if these values should be stored or directly put inside
     * a generator expression.
     */
    template <typename T>
    const double svg_renderer<T>::SVG_VERSION = 1.1;
    template <typename T>
    const std::string svg_renderer<T>::SVG_NAMESPACE_URL = "http://www.w3.org/2000/svg";

    template <typename T>
    svg_renderer<T>::svg_renderer(Map const& m, T & output_stream) :
	feature_style_processor<svg_renderer>(m),
	output_stream_(output_stream),
	width_(m.width()),
	height_(m.height())
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

	using repository::confix;
	using karma::format;
	using karma::lit;
	using karma::eol;
	using karma::int_;
	using karma::double_;
	using ascii::string;
	using ascii::space;

	// should I move these lines to the constructor?

	// generate XML header.
	output_stream_ << format(string << eol << string << eol, XML_DECLARATION, SVG_DTD);

	// generate SVG root element opening tag.
	output_stream_
	    << format(
		confix("<svg width=\"", "\"")[int_ << string],
		width_, "px")
	    << format(
		confix(" height=\"", "\"")[int_ << string],
		height_, "px")
	    << format(
		confix(" version=\"", "\"")[double_],
		SVG_VERSION)
	    << format(
		confix(" xmlns=\"", "\">")[string],
		SVG_NAMESPACE_URL);

	boost::optional<color> const& bgcolor = map.background();
	if(bgcolor)
	{
	    // generate background color as a rectangle that spans the whole image.
	    output_stream_
		<< format(
		    confix("\n<rect x=\"", "\"")[int_],
		    0)
		<< format(
		    confix(" y=\"", "\"")[int_],
		    0)
		<< format(
		    confix(" width=\"", "\"")[int_ << string],
		    width_, "px")
		<< format(
		    confix(" height=\"", "\"")[int_ << string],
		    height_, "px")
		<< format(
		    confix(" style=\"fill: ", "\"/>")[string],
		    bgcolor->to_hex_string());
	}
    }

    template <typename T>
    void svg_renderer<T>::end_map_processing(Map const& map)
    {
	using karma::format;
	using karma::lit;

	// generate SVG root element closing tag.
	// this doesn't work.
	output_stream_ << format(lit("\n</svg>"));

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

    template class svg_renderer<std::stringstream>;
}
