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

#include "polygon_symbolizer.hpp"

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba.h"
#include "agg_path_storage.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"

namespace mapnik
{
    polygon_symbolizer::polygon_symbolizer(const Color& fill)
	: symbolizer(),
	  fill_(fill) {}
    
    void polygon_symbolizer::render(geometry_type& geom,Image32& image) const 
    {
	typedef agg::renderer_base<agg::pixfmt_rgba32> ren_base;    
	typedef agg::renderer_scanline_aa_solid<ren_base> renderer;
	agg::row_ptr_cache<agg::int8u> buf(image.raw_data(),image.width(),image.height(),
					   image.width()*4);
	agg::pixfmt_rgba32 pixf(buf);
	ren_base renb(pixf);	    
	
	unsigned r=fill_.red();
	unsigned g=fill_.green();
	unsigned b=fill_.blue();
	unsigned a=fill_.alpha();
	renderer ren(renb);
	
	agg::rasterizer_scanline_aa<> ras;
	agg::scanline_u8 sl;
	ras.clip_box(0,0,image.width(),image.height());
	ras.add_path(geom);
	ren.color(agg::rgba8(r, g, b, a));
	agg::render_scanlines(ras, sl, ren);
    }
}
