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

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_conv_contour.h"
#include "agg_conv_stroke.h"
#include "agg_vcgen_stroke.h"
#include "agg_conv_adaptor_vcgen.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_conv_marker.h"
#include "agg_arrowhead.h"
#include "agg_vcgen_markers_term.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba32.h"
#include "agg_path_storage.h"
#include "agg_renderer_outline_aa.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_rasterizer_outline.h"
#include "agg_renderer_outline_image.h"

#include "symbolizer.hh"
#include "line_aa.hh"
#include "scanline_aa.hh"

namespace mapnik 
{
 
    struct LineSymbolizer : public SymbolizerImpl
    {
    private:
	Color pen_;
	double width_;
    public:
	LineSymbolizer(const Color& pen,double width=1.0)
	    : SymbolizerImpl(),
	      pen_(pen),
	      width_(width) {}

	LineSymbolizer(const Color& pen,double minScale,double maxScale,double width=1.0)
	    : SymbolizerImpl(minScale,maxScale),
	      pen_(pen),
	      width_(width) {}
        
	void render(geometry_type& geom, Image32& image) const 
	{
	    typedef agg::renderer_base<agg::pixfmt_rgba32> ren_base;    
	   
	    if (width_ == 1.0) 
	    {				
		//typedef agg::renderer_outline_aa<ren_base> renderer_oaa;
		//typedef agg::rasterizer_outline_aa<renderer_oaa> rasterizer_outline_aa;
		//agg::line_profile_aa prof;
		//prof.width(1.0);
		//renderer_oaa ren_oaa(ren_base, prof);
		//rasterizer_outline_aa ras_oaa(ren_oaa);
	  
		//ren_oaa.color(agg::rgba(r, g, b));
		//ras_oaa.add_path(geom);
		LineRasterizerAA<Image32> rasterizer(image);
		rasterizer.render<SHIFT0>(geom,pen_);
	    }
	    else 
	    {
		
		typedef agg::renderer_base<agg::pixfmt_rgba32> ren_base;    	
		agg::row_ptr_cache<agg::int8u> buf(image.raw_data(),image.width(),image.height(),image.width()*4);
		agg::pixfmt_rgba32 pixf(buf);
		ren_base renb(pixf);	    
		
		double r=pen_.red()/255.0;
		double g=pen_.green()/255.0;
		double b=pen_.blue()/255.0;
		
		typedef agg::renderer_scanline_aa_solid<ren_base> renderer;
		renderer ren(renb);
		
		
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;
		
		agg::conv_adaptor_vcgen<geometry<vertex2d,vertex_vector>,
		    agg::vcgen_stroke,agg::null_markers> stroke(geom);
		stroke.generator().line_join(agg::round_join);
		stroke.generator().line_cap(agg::round_cap);
		stroke.generator().miter_limit(2.0);
		stroke.generator().width(width_);

		//ScanlineRasterizerAA<Image32> rasterizer(image);
		ras.add_path(stroke);
		//rasterizer.render<agg::conv_adaptor_vcgen<geometry<vertex2d,vertex_vector>,
		//    agg::vcgen_stroke,agg::null_markers> >(stroke,pen_);
		ras.add_path(stroke);
		ren.color(agg::rgba(r, g, b));
		
		agg::render_scanlines(ras, sl, ren);
	    }
	    
	}
    private:
	LineSymbolizer(const LineSymbolizer&);
	LineSymbolizer& operator=(const LineSymbolizer&);
    };
}

#endif //LINE_SYMBOLIZER_HH
