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

#ifndef LINE_SYMBOLIZER_HH
#define LINE_SYMBOLIZER_HH

#include "symbolizer.hh"
#include "line_aa.hh"

namespace mapnik 
{
    struct LineSymbolizer : public SymbolizerImpl
    {
    private:
	Color pen_;
    public:
	LineSymbolizer(const Color& pen)
	    : SymbolizerImpl(),
	      pen_(pen) {}

	LineSymbolizer(const Color& pen,double minScale,double maxScale)
	    : SymbolizerImpl(minScale,maxScale),
	      pen_(pen) {}
        
	void render(const geometry_type& geom, Image32& image) const 
	{
	    LineRasterizerAA<Image32> rasterizer(image);
	    rasterizer.render<SHIFT0>(geom,pen_);
	}
    private:
	LineSymbolizer(const LineSymbolizer&);
	LineSymbolizer& operator=(const LineSymbolizer&);
    };
}

#endif //LINE_SYMBOLIZER_HH
