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

//$Id: polygon_symbolizer.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef POLYGON_SYMBOLIZER_HPP
#define POLYGON_SYMBOLIZER_HPP

#include "color.hpp"

namespace mapnik 
{
    struct MAPNIK_DECL polygon_symbolizer
    {
        explicit polygon_symbolizer() 
            : fill_(Color(128,128,128) {}

        polygon_symbolizer(Color const& fill)
            : fill_(fill) {}
        Color const& get_fill() const
        {
            return fill_;
        }
        void set_fill(Color const& fill)
        {
            fill_ = fill;
        }
    private:
        Color fill_;
    };  
}

#endif // POLYGON_SYMBOLIZER_HPP
