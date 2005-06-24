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

#include <iostream>

#ifndef COLORCUBE_HPP
#define COLORCUBE_HPP


namespace mapnik
{
    template <int rLevel,int gLevel,int bLevel>
    struct color_cube
    {
	const static unsigned maxcolors=rLevel * gLevel * bLevel + 32;	
	static unsigned cube[maxcolors];
	static bool initialized;
	static unsigned rDiv;
	static unsigned gDiv;
	static unsigned bDiv;
	
	static unsigned color(unsigned red,unsigned green,unsigned blue)
	{
	    if (!initialized)
		init();
	    unsigned index=rgb_level_index(red/rDiv,green/gDiv,blue/bDiv);
	    return cube[index];
	}
	
    private:
        static unsigned rgb_level_index(unsigned red,unsigned green,unsigned blue)
	{
	    return (red * gLevel * bLevel + green * bLevel + blue);
	} 
        
	static void init() 
	{
	    unsigned red,green,blue;
	    unsigned count=0;
	    for (int r=0;r<rLevel;++r)
	    {
		for (int g=0;g<gLevel;++g)
		{
		    for (int b=0;b<bLevel;++b)
		    {
			red = std::min(255,r * (255 /(rLevel - 1)));
			green = std::min(255,g * (255 /(gLevel - 1)));
			blue = std::min(255,b * (255 /(bLevel - 1)));
			
			cube[rgb_level_index(r,g,b)]= (red) |  (green<<8) | (blue<<16);
			//std::cout<<std::hex<<cube[rgb_level_index(r,g,b)]<<std::dec<<"\n";		        
			++count;
		    }
		}
	    }
	    
	    std::cout<<"number of colours="<<count<<"\n";
	}
    };

    //template<int rLevel,int gLevel,int bLevel>
    //unsigned color_cube<rLevel,gLevel,bLevel>::cube[color_cube<rLevel,gLevel, bLevel>::maxcolors];
    template<int rLevel,int gLevel,int bLevel>
    unsigned color_cube<rLevel,gLevel,bLevel>::rDiv=52;
    template<int rLevel,int gLevel,int bLevel>
    unsigned color_cube<rLevel,gLevel,bLevel>::gDiv=52;
    template<int rLevel,int gLevel,int bLevel>
    unsigned color_cube<rLevel,gLevel,bLevel>::bDiv=52;
    template<int rLevel,int gLevel,int bLevel>
    bool color_cube<rLevel,gLevel,bLevel>::initialized=false;
}

#endif 
