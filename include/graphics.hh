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

#ifndef GRAPHICS_HH
#define GRAPHICS_HH

#include <cmath>
#include <string>
#include <cassert>
#include "style.hh"
#include "gamma.hh"
#include "image_data.hh"

namespace mapnik
{
    class Image32
    {
    private:
	int width_;
	int height_;
	Color background_;
	ImageData32 data_;
	static gamma gammaTable_;
    public:
	Image32(int width,int height);
	Image32(const Image32& rhs);
	~Image32();
	static void setGamma(double gamma);
	int width() const;
	int height() const;
	void setBackground(const Color& background);
	const Color& getBackground() const;
	void set_rectangle(unsigned  x,unsigned y,const ImageData32& data);           
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

	inline bool checkBounds(int x,int y) const
	{
	    return (x>=0 && x<width_ && y>=0 && y<height_);
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
	    
	    return 0xff<<24 | r<<16 | g<<8 | b;
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
    };
}
#endif                                            //GRAPHICS_HH
