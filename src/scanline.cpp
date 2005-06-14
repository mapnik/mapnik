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

//$Id: scanline.cpp 17 2005-03-08 23:58:43Z pavlenko $

#include <vector>
#include "memory.hpp"
#include "graphics.hpp"
#include "scanline.hpp"

namespace mapnik
{

    template <typename PixBuffer>
    template <typename Transform>
    void ScanlineRasterizer<PixBuffer>::render(const geometry_type& geom,const Color& c)
    {
	// ????
    }

    template <typename PixBuffer>
    inline void ScanlineRasterizer<PixBuffer>::render_hline(int x0,int x1,int y,unsigned int rgba)
    {
        int x;
        if (x0<0) x0=0;
        if (x1> (int)pixbuf_->width()-1) x1=pixbuf_->width()-1;
        for(x=x0;x<=x1;x++) pixbuf_->setPixel(x,y,rgba);
    }

    template class ScanlineRasterizer<Image32>;
}
