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

#include "stroke.hpp"

namespace mapnik 
{
    struct MAPNIK_DECL line_symbolizer
    {
	line_symbolizer(stroke const& stroke)
	    : stroke_(stroke) {}
	
	line_symbolizer(const Color& pen,float width=1.0)
	    : stroke_(pen,width) {}
	stroke const& get_stroke() const
	{
	    return stroke_;
	}
	void set_stroke(stroke const& stroke)
	{
	    stroke_ = stroke;
	}

    private:
		stroke stroke_;
    };
}

#endif //LINE_SYMBOLIZER_HPP
