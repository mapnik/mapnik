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

//$Id: polygon_symbolizer.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef POLYGON_SYMBOLIZER_HPP
#define POLYGON_SYMBOLIZER_HPP

#include "symbolizer.hpp"
#include "scanline_aa.hpp"
#include "line_aa.hpp"

namespace mapnik 
{
    struct polygon_symbolizer : public symbolizer
    {
    private:
	Color fill_;
    public:
	polygon_symbolizer(const Color& fill)
	    : symbolizer(),
	      fill_(fill) {}
	
	virtual ~polygon_symbolizer() {}

	void render(geometry_type& geom,Image32& image) const 
	{
	    ScanlineRasterizerAA<Image32> rasterizer(image);
	    rasterizer.render<SHIFT8>(geom,fill_);
	}
	
    private:
	polygon_symbolizer(const polygon_symbolizer&);
	polygon_symbolizer& operator=(const polygon_symbolizer&);
	
    };
}

#endif // POLYGON_SYMBOLIZER_HPP
