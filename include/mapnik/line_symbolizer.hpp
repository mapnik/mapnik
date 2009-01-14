/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/
//$Id: line_symbolizer.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef LINE_SYMBOLIZER_HPP
#define LINE_SYMBOLIZER_HPP

#include <mapnik/stroke.hpp>

namespace mapnik 
{
    struct MAPNIK_DECL line_symbolizer
    {
        explicit line_symbolizer()
            : stroke_() {}
        
        line_symbolizer(stroke const& stroke)
            : stroke_(stroke) {}
	
        line_symbolizer(color const& pen,float width=1.0)
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
