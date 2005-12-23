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

//$Id: line_symbolizer.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef LINE_SYMBOLIZER_HPP
#define LINE_SYMBOLIZER_HPP

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

#include "symbolizer.hpp"
#include "stroke.hpp"

#include <boost/utility.hpp>

namespace mapnik 
{
    struct line_symbolizer : public symbolizer,
			     private boost::noncopyable
    {
    private:
	stroke stroke_;
    public:

	line_symbolizer(stroke const& stroke)
	    : symbolizer(),
	      stroke_(stroke) {}

	line_symbolizer(const Color& pen,float width=1.0)
	    : symbolizer(),
	      stroke_(pen,width) {}
	
	void render(geometry_type& geom, Image32& image) const 
	{
	    typedef agg::renderer_base<agg::pixfmt_rgba32> ren_base;    
	    agg::row_ptr_cache<agg::int8u> buf(image.raw_data(),image.width(),image.height(),
					       image.width()*4);
	    agg::pixfmt_rgba32 pixf(buf);
	    ren_base renb(pixf);	    
	    
	    Color const& col = stroke_.get_color();
	    double r=col.red()/255.0;
	    double g=col.green()/255.0;
	    double b=col.blue()/255.0;
	    

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
		ren.color(agg::rgba(r, g, b, stroke_.get_opacity()));
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
		ren.color(agg::rgba(r, g, b, stroke_.get_opacity()));
		agg::render_scanlines(ras, sl, ren);
	    }
	}
    };
}

#endif //LINE_SYMBOLIZER_HPP
