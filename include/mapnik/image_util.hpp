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

//$Id: image_util.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef IMAGE_UTIL_HPP
#define IMAGE_UTIL_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/graphics.hpp>
// stl
#include <string>

namespace mapnik {
    
    template <typename T>
    MAPNIK_DECL void save_to_file(std::string const& filename,
                      std::string const& type,
                      T const& image);
    template <typename T>
    void save_as_png(std::string const& filename,
		     T const& image);

    template <typename T>
    void save_as_jpeg(std::string const& filename,
		      int quality,
		      T const& image);

    template <typename T>
    double distance(T x0,T y0,T x1,T y1)
    {
        double dx = x1-x0;
        double dy = y1-y0;
        return sqrt(dx * dx + dy * dy);
    }

    template <typename Image>
    inline void scale_down2(Image& target,const Image& source)
    {
        int source_width=source.width();
        int source_height=source.height();

        int target_width=target.width();
        int target_height=target.height();
        if (target_width<source_width/2 || target_height<source_height/2)
            return;
        int y1,x1;
        for (int y=0;y<target_height;++y)
        {
            y1=2*y;
            for(int x=0;x<target_width;++x)
            {
                x1=2*x;
                //todo calculate average???
                target(x,y)=source(x1,y1);
            }
        }
    }

    template <typename Image,int scale>
    struct image_op
    {
        static void scale_up(Image& target,const Image& source)
        {
            if (scale<3) return;
            int source_width=source.width();
            int source_height=source.height();

            int target_width=target.width();
            int target_height=target.height();
            if (target_width<scale*source_width || target_height<scale*source_height)
                return;
            for (int y=0;y<source_height;++y)
            {
                for(int x=0;x<source_width;++x)
                {
                    unsigned p=source(x,y);
                    for (int i=0;i<scale;++i)
                        for (int j=0;j<scale;++j)
                            target(scale*x+i,scale*y+j)=p;
                }
            }
        }
    };

    template <typename Image>
    struct image_op<Image,2>
    {
        static void scale_up(Image& target,const Image& source)
        {
            int source_width=source.width();
            int source_height=source.height();

            int target_width=target.width();
            int target_height=target.height();
            if (target_width<2*source_width || target_height<2*source_height)
                return;
            for (int y=0;y<source_height;++y)
            {
                for(int x=0;x<source_width;++x)
                {
                    target(2*x,2*y)=source(x,y);
                    target(2*x+1,2*y)=source(x,y);
                    target(2*x+1,2*y+1)=source(x,y);
                    target(2*x,2*y+1)=source(x,y);
                }
            }
        }
    };

    namespace
    {
        template <typename Image>
        inline void scale_up(Image& target,const Image& source,unsigned scale)
        {
            int source_width=source.width();
            int source_height=source.height();

            int target_width=target.width();
            int target_height=target.height();
            if (target_width<scale*source_width || target_height<scale*source_height)
                return;
            for (int y=0;y<source_height;++y)
            {
                for(int x=0;x<source_width;++x)
                {
                    unsigned p=source(x,y);
                    for (int i=0;i<scale;++i)
                        for (int j=0;j<scale;++j)
                            target(scale*x+i,scale*y+j)=p;
                }
            }
        }
    }
    
    template <typename Image>
    void scale_image(Image& target,const Image& source,unsigned scale)
    {
        if (scale==2)
        {
            image_op<Image,2>::scale_up(target,source);
        }
        else
        {
            scale_up<Image>(target,source,scale);
        }
    }

    template <typename Image>
    inline void scale_image (Image& target,const Image& source)
    {

        int source_width=source.width();
        int source_height=source.height();

        int target_width=target.width();
        int target_height=target.height();

        if (source_width<1 || source_height<1 ||
            target_width<1 || target_height<1) return;
        int int_part_y=source_height/target_height;
        int fract_part_y=source_height%target_height;
        int err_y=0;
        int int_part_x=source_width/target_width;
        int fract_part_x=source_width%target_width;
        int err_x=0;
        int x=0,y=0,xs=0,ys=0;
        int prev_y=-1;
        for (y=0;y<target_height;++y)
        {
            if (ys==prev_y)
            {
                target.setRow(y,target.getRow(y-1),target_width);
            }
            else
            {
                xs=0;
                for (x=0;x<target_width;++x)
                {
                    target(x,y)=source(xs,ys);
                    xs+=int_part_x;
                    err_x+=fract_part_x;
                    if (err_x>=target_width)
                    {
                        err_x-=target_width;
                        ++xs;
                    }
                }
                prev_y=ys;
            }
            ys+=int_part_y;
            err_y+=fract_part_y;
            if (err_y>=target_height)
            {
                err_y-=target_height;
                ++ys;
            }
        }
    }
   
   inline MAPNIK_DECL void save_to_file (std::string const& file,
                                         std::string const& type,
                                         Image32 const& image) 
   {
      save_to_file<ImageData32>(file,type,image.data());
   }
   
#ifdef _MSC_VER
  template MAPNIK_DECL void save_to_file<ImageData32>(std::string const&,
						      std::string const& ,
                                                      ImageData32 const&);
         
  template MAPNIK_DECL void save_to_file<image_view<ImageData32> > (std::string const&,
                                                                    std::string const& ,
                                                                    image_view<ImageData32> const&);
#endif

}

#endif //IMAGE_UTIL_HPP
