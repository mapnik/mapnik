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

//$Id: graphics.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <cmath>
#include <string>
#include <cassert>
#include "color.hpp"
#include "gamma.hpp"
#include "image_data.hpp"
#include "envelope.hpp"

namespace mapnik
{
    class Image32
    {
    private:
	unsigned width_;
	unsigned height_;
	Color background_;
	ImageData32 data_;
	static gamma gammaTable_;
    public:
	Image32(int width,int height);
	Image32(const Image32& rhs);
	~Image32();
	static void setGamma(double gamma);
	void setBackground(const Color& background);
	const Color& getBackground() const;     
	const ImageData32& data() const;
	inline ImageData32& data() {
	    return data_;
	}
	inline const unsigned char* raw_data() const
	{
	    return data_.getBytes();
	}
	
	inline unsigned char* raw_data()
	{
	    return data_.getBytes();
	}
	
	void saveToFile(const std::string& file,const std::string& format="auto"); 
    private:

	inline bool checkBounds(unsigned x, unsigned y) const
	{
	    return (x < width_ && y < height_);
	}

    public:
	inline void setPixel(int x,int y,unsigned int rgba)
	{
	    if (checkBounds(x,y))
	    {
		data_(x,y)=rgba;
	    }
	}
	inline int blendColor(int c0,int c1,int t)
	{
	    int bgRed=(c1>>16)&0xff;
	    int bgGreen=(c1>>8)&0xff;
	    int bgBlue=c1&0xff;

	    int red=(c0>>16)&0xff;
	    int green=(c0>>8)&0xff;
	    int blue=c0&0xff;

	    int alpha=t;

	    int r=gammaTable_.l2g[(gammaTable_.g2l[red]*alpha+gammaTable_.g2l[bgRed]*(255-alpha))>>8];
	    int g=gammaTable_.l2g[(gammaTable_.g2l[green]*alpha+gammaTable_.g2l[bgGreen]*(255-alpha))>>8];
	    int b=gammaTable_.l2g[(gammaTable_.g2l[blue]*alpha+gammaTable_.g2l[bgBlue]*(255-alpha))>>8];
	    
	    return 0xff << 24 | r << 16 | g << 8 | b;
	}

	inline void blendPixel(int x,int y,unsigned int rgba,int t)
	{
	    if (checkBounds(x,y))
	    {
		int bg=data_(x,y);
		int nc=blendColor(rgba,bg,t);
		data_(x,y)=nc;
	    }
	}

	inline unsigned width() const
	{
	    return width_;
	}
	
	inline unsigned height() const
	{
	    return height_;
	}

	inline void set_rectangle(int x0,int y0,ImageData32 const& data)
	{
	    Envelope<int> ext0(0,0,width_,height_);   
	    Envelope<int> ext1(x0,y0,x0+data.width(),y0+data.height());
	    
	    if (ext0.intersects(ext1))
	    {	
		Envelope<int> box = ext0.intersect(ext1);
		for (int y = box.miny(); y < box.maxy(); ++y)
		{
		    for (int x = box.minx(); x < box.maxx(); ++x)
		    {
			if ((data(x-x0,y-y0) & 0xff000000)) 
			{
			    data_(x,y)=data(x-x0,y-y0);
			}
		    }
		}   
	    }
	}
	
	inline void set_rectangle_alpha(int x0,int y0,const ImageData32& data)
	{
	    Envelope<int> ext0(0,0,width_,height_);   
	    Envelope<int> ext1(x0,y0,x0 + data.width(),y0 + data.height());
	    
	    if (ext0.intersects(ext1))
	    {	                		
		Envelope<int> box = ext0.intersect(ext1);		
		for (int y = box.miny(); y < box.maxy(); ++y)
		{
		    for (int x = box.minx(); x < box.maxx(); ++x)
		    {
			unsigned rgba0 = data_(x,y);
			unsigned rgba1 = data(x-x0,y-y0);
		    
			unsigned a1 = (rgba1 >> 24) & 0xff;
			if (a1 == 0) continue;
			unsigned r1 = rgba1 & 0xff;
			unsigned g1 = (rgba1 >> 8 ) & 0xff;
			unsigned b1 = (rgba1 >> 16) & 0xff;
		    
			unsigned a0 = (rgba0 >> 24) & 0xff;
			unsigned r0 = (rgba0 & 0xff) * a0;
			unsigned g0 = ((rgba0 >> 8 ) & 0xff) * a0;
			unsigned b0 = ((rgba0 >> 16) & 0xff) * a0;
		    
		    
			a0 = ((a1 + a0) << 8) - a0*a1;
		    
			r0 = ((((r1 << 8) - r0) * a1 + (r0 << 8)) / a0);
			g0 = ((((g1 << 8) - g0) * a1 + (g0 << 8)) / a0);
			b0 = ((((b1 << 8) - b0) * a1 + (b0 << 8)) / a0);
			a0 = a0 >> 8;
			data_(x,y)= (a0 << 24)| (b0 << 16) |  (g0 << 8) | (r0) ;
		    }
		}
	    }
	}
    };
}
#endif //GRAPHICS_HPP
