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

//$Id: line_aa.cpp 29 2005-04-01 14:30:11Z pavlenko $

#include "line_aa.hpp"
#include "geom_util.hpp"

namespace mapnik
{
    using std::swap;

    template <typename PixBuffer>
    template <typename Transform>
    void LineRasterizerAA<PixBuffer>::render(const geometry_type& path,const Color& c)
    {
        unsigned rgba=c.rgba();
        typename geometry_type::path_iterator<Transform> itr=path.template begin<Transform>();
         
	int x0 = 0 ,y0 = 0;
        while (itr!=path.template end<Transform>())
        {
            int x1=itr->x;
            int y1=itr->y;
            if (itr->cmd==SEG_LINETO || itr->cmd==SEG_CLOSE)
            {
                if (!(x0==x1 && y0==y1))
                {
                    render_line(x0,y0,x1,y1,rgba);
                }
	    }
            x0=x1;
            y0=y1;
            ++itr;
        }
    }

    template <typename PixBuffer>
    inline void LineRasterizerAA<PixBuffer>::render_line(int x0,int y0,int x1,int y1,unsigned rgba)
    {
        if (!clip_line(x0,y0,x1,y1,pixbuf_)) return;
        if(y0>y1)
        {
            swap(y0,y1);
            swap(x0,x1);
        }
        pixbuf_->setPixel(x0,y0,rgba);
        pixbuf_->setPixel(x1,y1,rgba);
        int dx=x1-x0;
        int dy=y1-y0;
        int xDir;
        if(dx>=0) xDir=1;
        else
        {
            xDir=-1;
            dx=-dx;
        }
        if(dx==0)                                 // vertical line
        {
            for(int y=y0;y<y1;y++)
            {
                pixbuf_->setPixel(x0,y,rgba);
            }
            return;
        }
        if(dy==0)                                 // horizontal line
        {
            if (x0>x1) swap(x0,x1);
            for(int x=x0;x<x1;++x)
            {
                pixbuf_->setPixel(x,y0,rgba);
            }
            return;
        }
        if(dx==dy)                                // diagonal line.
        {
            for(int x=x0,y=y0;y<y1;y++,x+=xDir)
            {
                pixbuf_->setPixel(x,y,rgba);
            }
            return;
        }
        // line is not horizontal, diagonal, or vertical: use Wu Antialiasing:
        int error_acc=0;
        int t;
        if(dy>dx)                                 // y-major line
        {
            int error_adj=(dx<<16)/dy;
            if(xDir<0)
            {
                while(--dy)
                {
                    error_acc+=error_adj;
                    ++y0;
                    x1=x0-(error_acc>>16);
                    t=(error_acc>>8)&255;
                    pixbuf_->blendPixel(x1  , y0, rgba,~t&255);
                    pixbuf_->blendPixel(x1-1, y0, rgba,t);
                }
            }
            else
            {
                while(--dy)
                {
                    error_acc+=error_adj;
                    ++y0;
                    x1=x0+(error_acc>>16);
                    t=(error_acc>>8)&255;
                    pixbuf_->blendPixel(x1     , y0, rgba,~t&255);
                    pixbuf_->blendPixel(x1+xDir, y0, rgba,t);
                }
            }
        }                                         // x-major line
        else
        {
            int error_adj=(dy<<16)/dx;
            while(--dx)
            {
                error_acc+=error_adj;
                x0+=xDir;
                y1=y0+(error_acc>>16);
                t=(error_acc>>8)&255;
                pixbuf_->blendPixel(x0, y1  , rgba,~t&255);
                pixbuf_->blendPixel(x0, y1+1, rgba,t);
            }
        }
    }

    template class LineRasterizerAA<Image32>;
    template void LineRasterizerAA<Image32>::render<SHIFT0>(const geometry_type&,const Color&);
}
