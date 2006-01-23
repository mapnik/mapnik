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

#include "image_symbolizer.hpp"
#include "image_data.hpp"
#include "image_reader.hpp"

namespace mapnik
{
    image_symbolizer::image_symbolizer(std::string const& file,
			 std::string const& type,
			 unsigned width,unsigned height) 
	    : symbolizer(),
	      symbol_(width,height)
	{
	    try 
	    {
		std::auto_ptr<ImageReader> reader(get_image_reader(type,file));
		reader->read(0,0,symbol_);		
	    } 
	    catch (...) 
	    {
		std::cerr<<"exception caught..." << std::endl;
	    }
	}
  
    void image_symbolizer::render(geometry_type& geom,Image32& image) const 
    {
	int w=symbol_.width();
	int h=symbol_.height();
	double x,y;
	unsigned size = geom.num_points();
	for (unsigned pos = 0; pos < size ;++pos)
	{
	    geom.vertex(&x,&y);
	    int px=int(x - 0.5 * w);
	    int py=int(y - 0.5 * h);
	    image.set_rectangle_alpha(px,py,symbol_);
	}
    }
}

