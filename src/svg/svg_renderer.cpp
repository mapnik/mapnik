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
#include <boost/spirit/include/karma_string.hpp>

namespace karma = boost::spirit::karma;

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

    template <typename T>
    svg_renderer<T>::svg_renderer(Map const& m, T & output_stream) :
	feature_style_processor<svg_renderer>(m),
	output_stream_(output_stream)
    {
	// nothing yet.
    }

    template <typename T>
    svg_renderer<T>::~svg_renderer() {}

    // only empty methods for now.

    template <typename T>
    void svg_renderer<T>::start_map_processing(Map const& map)
    {
	#ifdef MAPNIK_DEBUG
	std::clog << "start map processing" << std::endl;
	#endif

	output_stream_ << karma::format(karma::lit(XML_DECLARATION) << karma::eol << karma::lit(SVG_DTD), "");	
    }

    template <typename T>
    void svg_renderer<T>::end_map_processing(Map const& map)
    {
	// nothing yet.

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
