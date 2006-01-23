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

#ifndef LINE_PATTERN_SYMBOLIZER_HPP
#define LINE_PATTERN_SYMBOLIZER_HPP

#include "symbolizer.hpp"
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
#include "agg_pattern_filters_rgba.h"

#include <boost/utility.hpp>

namespace mapnik 
{
    class pattern_source : private boost::noncopyable
    {
    public:
	pattern_source(ImageData32 const& pattern)
	    : pattern_(pattern) {}
	
	unsigned int width() const
	{
	    return pattern_.width();
	}
	unsigned int height() const
	{
	    return pattern_.height();
	}
	agg::rgba8 pixel(int x, int y) const
	{
	    unsigned c = pattern_(x,y);
	    return agg::rgba8(c & 0xff, (c >> 8) & 0xff, (c >> 16) & 0xff,(c >> 24) & 0xff);
	}
    private:
	ImageData32 const& pattern_;
    };

    struct line_pattern_symbolizer : public symbolizer, 
				     private boost::noncopyable
    {
    private:
	ImageData32 pattern_;
    public:
	line_pattern_symbolizer(std::string const& file,
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

	virtual ~line_pattern_symbolizer() {}

	void render(geometry_type& geom,Image32& image) const
	{
	    typedef agg::line_image_pattern<agg::pattern_filter_bilinear_rgba8> pattern_type;
	    typedef agg::renderer_base<agg::pixfmt_rgba32> renderer_base;
	    typedef agg::renderer_outline_image<renderer_base, pattern_type> renderer_type;
	    typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;
	    unsigned int width=image.width();
	    unsigned int height=image.height();
	    agg::row_ptr_cache<agg::int8u> buf(image.raw_data(), width, height,width*4);
	    agg::pixfmt_rgba32 pixf(buf);
	    renderer_base ren_base(pixf);  
	    agg::pattern_filter_bilinear_rgba8 filter; 
	    pattern_source source(pattern_);
	    pattern_type pattern (filter,source);
	    renderer_type ren(ren_base, pattern);
	    ren.clip_box(0,0,width,height);
	    rasterizer_type ras(ren);	    
	    ras.add_path(geom);
	}
    };    
}

#endif // LINE_PATTERN_SYMBOLIZER_HPP
