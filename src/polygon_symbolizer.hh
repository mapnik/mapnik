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

#ifndef POLYGON_SYMBOLIZER_HH
#define POLYGON_SYMBOLIZER_HH


#include "symbolizer.hh"
#include "scanline_aa.hh"
#include "line_aa.hh"

namespace mapnik 
{
    struct PolygonSymbolizer : public SymbolizerImpl
    {
    private:
	Color fill_;
    public:
	PolygonSymbolizer(const Color& fill)
	    : SymbolizerImpl(),
	      fill_(fill) {}
	
	PolygonSymbolizer(const Color& fill,double min_scale,double max_scale) 
	    : SymbolizerImpl(min_scale,max_scale),
	      fill_(fill)
	{}
	
	virtual ~PolygonSymbolizer() {}

	void render(const geometry_type& geom,Image32& image) const 
	{
	    ScanlineRasterizerAA<Image32> rasterizer(image);
	    rasterizer.render<SHIFT8>(geom,fill_);
	}
	
    private:
	PolygonSymbolizer(const PolygonSymbolizer&);
	PolygonSymbolizer& operator=(const PolygonSymbolizer&);
	
    };
    
    struct OutlinedPolygonSymbolizer : public SymbolizerImpl 
    {
    private:
	Color fill_;
	Color stroke_;
    public:
	OutlinedPolygonSymbolizer(const Color& fill,const Color& stroke)
	    : SymbolizerImpl(),
	      fill_(fill),
	      stroke_(stroke) {}

	OutlinedPolygonSymbolizer(const Color& fill,const Color& stroke,double minScale,double maxScale)
	    : SymbolizerImpl(minScale,maxScale),
	      fill_(fill),
	      stroke_(stroke) {}
	
	void render(const geometry_type& geom,Image32& image) const 
	{
	    ScanlineRasterizerAA<Image32> rasterizer1(image);
	    rasterizer1.render<SHIFT8>(geom,fill_);
	    LineRasterizerAA<Image32> rasterizer2(image);
	    rasterizer2.render<SHIFT0>(geom,stroke_);
	}
	
    private:
	OutlinedPolygonSymbolizer(const OutlinedPolygonSymbolizer&);
	OutlinedPolygonSymbolizer& operator=(const OutlinedPolygonSymbolizer&);
    };
}

#endif // POLYGON_SYMBOLIZER_HH
