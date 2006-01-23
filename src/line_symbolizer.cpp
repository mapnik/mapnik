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

#include "line_symbolizer.hpp"

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_contour.h"
#include "agg_vcgen_stroke.h"
#include "agg_conv_adaptor_vcgen.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_conv_marker.h"
#include "agg_arrowhead.h"
#include "agg_vcgen_markers_term.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba.h"
#include "agg_path_storage.h"
#include "agg_renderer_outline_aa.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_rasterizer_outline.h"
#include "agg_renderer_outline_image.h"

namespace mapnik
{
    line_symbolizer::line_symbolizer(stroke const& stroke)
	: symbolizer(),
	  stroke_(stroke) {}
    
    line_symbolizer::line_symbolizer(const Color& pen,float width)
	    : symbolizer(),
	      stroke_(pen,width) {}
	
    void line_symbolizer::render(geometry_type& geom, Image32& image) const 
    {
	typedef agg::renderer_base<agg::pixfmt_rgba32> ren_base;    
	agg::row_ptr_cache<agg::int8u> buf(image.raw_data(),image.width(),image.height(),
					   image.width()*4);
	agg::pixfmt_rgba32 pixf(buf);
	ren_base renb(pixf);	    
	
	Color const& col = stroke_.get_color();
	unsigned r=col.red();
	unsigned g=col.green();
	unsigned b=col.blue();	    
	
	if (0)//stroke_.get_width() <= 1.0)
	{
	    typedef agg::renderer_outline_aa<ren_base> renderer_oaa;
	    typedef agg::rasterizer_outline_aa<renderer_oaa> rasterizer_outline_aa;
	    agg::line_profile_aa prof;
	    prof.width(stroke_.get_width());
	    renderer_oaa ren_oaa(renb, prof);
	    rasterizer_outline_aa ras_oaa(ren_oaa);
	    
	    ren_oaa.color(agg::rgba8(r, g, b, int(255*stroke_.get_opacity())));
	    ren_oaa.clip_box(0,0,image.width(),image.height());
	    ras_oaa.add_path(geom);		
	    
	}
	else 
	{
	    typedef agg::renderer_scanline_aa_solid<ren_base> renderer;
	    renderer ren(renb);	
	    agg::rasterizer_scanline_aa<> ras;
	    agg::scanline_u8 sl;
	    
	    if (stroke_.has_dash())
	    {
		
		agg::conv_dash<geometry<vertex2d> > dash(geom);
		dash_array const& d = stroke_.get_dash_array();
		dash_array::const_iterator itr = d.begin();
		dash_array::const_iterator end = d.end();
		while (itr != end)
		{
		    dash.add_dash(itr->first, itr->second);
		    ++itr;
		}
		agg::conv_stroke<agg::conv_dash<geometry<vertex2d> > > stroke(dash);
		
		line_join_e join=stroke_.get_line_join();
		if ( join == MITER_JOIN)
		    stroke.generator().line_join(agg::miter_join);
		else if( join == MITER_REVERT_JOIN) 
		    stroke.generator().line_join(agg::miter_join);
		else if( join == ROUND_JOIN) 
		    stroke.generator().line_join(agg::round_join);
		else
		    stroke.generator().line_join(agg::bevel_join);
		    
		line_cap_e cap=stroke_.get_line_cap();
		if (cap == BUTT_CAP)    
		    stroke.generator().line_cap(agg::butt_cap);
		else if (cap == SQUARE_CAP)
		    stroke.generator().line_cap(agg::square_cap);
		else 
		    stroke.generator().line_cap(agg::round_cap);

		stroke.generator().miter_limit(4.0);
		stroke.generator().width(stroke_.get_width());
		
		ras.clip_box(0,0,image.width(),image.height());
		ras.add_path(stroke);
		ren.color(agg::rgba8(r, g, b, int(255*stroke_.get_opacity())));
		agg::render_scanlines(ras, sl, ren);
	    }
	    else 
	    {
		agg::conv_stroke<geometry<vertex2d> >  stroke(geom);
		
		line_join_e join=stroke_.get_line_join();
		if ( join == MITER_JOIN)
		    stroke.generator().line_join(agg::miter_join);
		else if( join == MITER_REVERT_JOIN) 
		    stroke.generator().line_join(agg::miter_join);
		else if( join == ROUND_JOIN) 
		    stroke.generator().line_join(agg::round_join);
		else
		    stroke.generator().line_join(agg::bevel_join);
		    
		line_cap_e cap=stroke_.get_line_cap();
		if (cap == BUTT_CAP)    
		    stroke.generator().line_cap(agg::butt_cap);
		else if (cap == SQUARE_CAP)
		    stroke.generator().line_cap(agg::square_cap);
		else 
		    stroke.generator().line_cap(agg::round_cap);

		stroke.generator().miter_limit(4.0);
		stroke.generator().width(stroke_.get_width());
		    
		ras.clip_box(0,0,image.width(),image.height());
		ras.add_path(stroke);
		ren.color(agg::rgba8(r, g, b, int(255*stroke_.get_opacity())));
		agg::render_scanlines(ras, sl, ren);
	    }
	}
    }
}
