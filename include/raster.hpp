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

//$Id: raster.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef RASTER_HPP
#define RASTER_HPP

#include "graphics.hpp"

namespace mapnik
{
    struct raster
    {
        int x_;
        int y_;
        ImageData32 data_;
	raster(int x,int y,ImageData32& data)
	    : x_(x),
	      y_(y),
	      data_(data) {}
    };
}

#endif //RASTER_HPP
