/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: render.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef RENDER_HPP
#define RENDER_HPP

#include <stack>
#include "memory.hpp"
#include "ptr.hpp"
#include "style.hpp"
#include "envelope.hpp"
#include "graphics.hpp"
#include "datasource.hpp"
#include "layer.hpp"
#include "map.hpp"

namespace mapnik
{
    template <typename Image> class Renderer
    {
    public:
	static void render(const Map& map,Image& image);
    private:
	Renderer();
	static void render_vector_layer(datasource_p const& ds,
					std::vector<std::string> const& , 
					unsigned width,
					unsigned height,
					const Envelope<double>& bbox,Image& image);
	static void render_raster_layer(datasource_p const& ds,
					std::vector<std::string> const& namedStyles,
					unsigned width,
					unsigned height,
					const Envelope<double>& bbox,Image& image);
    };
}

#endif //RENDER_HPP
