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

namespace mapnik
{
    svg_renderer::svg_renderer(Map const& m) :
	feature_style_processor<svg_renderer>(m)
    {
	// nothing yet.
    }

    svg_renderer::~svg_renderer() {}

    // only empty methods for now.

    void svg_renderer::start_map_processing(Map const& map)
    {
	// nothing yet.

	#ifdef MAPNIK_DEBUG
	std::clog << "start map processing" << std::endl;
	#endif
    }

    void svg_renderer::end_map_processing(Map const& map)
    {
	// nothing yet.

	#ifdef MAPNIK_DEBUG
	std::clog << "end map processing" << std::endl;
	#endif
    }

    void svg_renderer::start_layer_processing(layer const& lay)
    {
	// nothing yet.

	#ifdef MAPNIK_DEBUG
	std::clog << "start layer processing: " << lay.name() << std::endl;
	#endif
    }
    
    void svg_renderer::end_layer_processing(layer const& lay)
    {
	// nothing yet.

	#ifdef MAPNIK_DEBUG
	std::clog << "end layer processing: " << lay.name() << std::endl;
	#endif
    }
}
