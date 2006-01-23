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

#include "polygon_pattern_symbolizer.hpp"

#include "image_reader.hpp"
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
    polygon_pattern_symbolizer::polygon_pattern_symbolizer(std::string const& file,
							   std::string const& type,
							   unsigned width,unsigned height) 
	: symbolizer(),
	  pattern_(width,height)
    {
	try 
	{
	    std::auto_ptr<ImageReader> reader(get_image_reader(type,file));
	    reader->read(0,0,pattern_);		
	} 
	catch (...) 
	{
	    std::cerr<<"exception caught..."<<std::endl;
	}
    }
    
    void polygon_pattern_symbolizer::render(geometry_type& geom,Image32& image) const
    {
	typedef agg::renderer_base<agg::pixfmt_rgba32> ren_base; 
	agg::row_ptr_cache<agg::int8u> buf(image.raw_data(),image.width(),image.height(),
					   image.width()*4);
	agg::pixfmt_rgba32 pixf(buf);
	ren_base renb(pixf);
	
	unsigned w=pattern_.width();
	unsigned h=pattern_.height();
	agg::row_ptr_cache<agg::int8u> pattern_rbuf((agg::int8u*)pattern_.getBytes(),w,h,w*4);  
	
	typedef agg::wrap_mode_repeat wrap_x_type;
	typedef agg::wrap_mode_repeat wrap_y_type;
	typedef agg::image_accessor_wrap<agg::pixfmt_rgba32, 
	    wrap_x_type,
	    wrap_y_type> img_source_type;
	
	typedef agg::span_pattern_rgba<img_source_type> span_gen_type;
	
	typedef agg::renderer_scanline_aa<ren_base, 
	    agg::span_allocator<agg::rgba8>,
	    span_gen_type> renderer_type;  
	
	double x0,y0;
	geom.vertex(&x0,&y0);
	geom.rewind(0);
	
	unsigned offset_x = unsigned(image.width() - x0);
	unsigned offset_y = unsigned(image.height() - y0);
	
	agg::span_allocator<agg::rgba8> sa;
	img_source_type img_src(pattern_rbuf);
	span_gen_type sg(img_src, offset_x, offset_y);
	renderer_type rp(renb,sa, sg);
	
	agg::rasterizer_scanline_aa<> ras;
	agg::scanline_u8 sl;
	ras.clip_box(0,0,image.width(),image.height());
	ras.add_path(geom);
	agg::render_scanlines(ras, sl, rp);   
    }
}
