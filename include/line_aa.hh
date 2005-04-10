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

#ifndef LINE_AA_HH
#define LINE_AA_HH

#include "geometry.hh"
#include "graphics.hh"
#include "style.hh"

namespace mapnik
{
    template <typename PixBuffer> class LineRasterizerAA
    {
    private:
	PixBuffer* pixbuf_;
    public:
	LineRasterizerAA(PixBuffer& pixbuf)
                :pixbuf_(&pixbuf) {}
	
	template <typename Transform>
	void render(const geometry_type& geom,const Color& c);
    private:
	LineRasterizerAA(const LineRasterizerAA&);
	LineRasterizerAA& operator=(const LineRasterizerAA&);
	void render_line(int x0,int y0,int x1,int y1,unsigned rgba);
    };
}
#endif                                            //LINE_AA_HH
