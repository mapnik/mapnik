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

#ifndef SYMBOLIZER_HH
#define SYMBOLIZER_HH

#include "graphics.hh" 
#include "geometry.hh"
#include <limits>

namespace mapnik 
{
    class Image32;
    struct Symbolizer
    {
    	virtual bool active(double scale) const=0;
    	virtual void render(geometry_type& geom, Image32& image) const=0;
    	virtual ~Symbolizer() {}
    };
    
    struct SymbolizerImpl : public Symbolizer
    {
    private:
	double min_scale_;
	double max_scale_;	
    public:
	SymbolizerImpl()
	    : min_scale_(0),
	      max_scale_(std::numeric_limits<double>::max()) {}
	SymbolizerImpl(double min_scale,double max_scale) 
	    : min_scale_(min_scale),
	      max_scale_(max_scale) {}
	
	virtual ~SymbolizerImpl() {}
	
	bool active(double scale) const
	{
	    return ( scale > min_scale_ && scale < max_scale_ ); 
	}
    };
}

#endif //SYMBOLIZER_HH
