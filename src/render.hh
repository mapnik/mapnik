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

//$Id$

#ifndef RENDER_HH
#define RENDER_HH

#include <stack>
#include "memory.hh"
#include "ptr.hh"
#include "style.hh"
#include "envelope.hh"
#include "graphics.hh"
#include "datasource.hh"
#include "layer.hh"
#include "map.hh"

namespace mapnik
{
    template <typename Image> class Renderer
    {
    public:
	static void render(const Map& map,Image& image);
    private:
	Renderer();
	static void renderLayer(const Layer& l,const CoordTransform& t,const query& q,Image& image);
    };
}

#endif                                            //RENDER_HH
