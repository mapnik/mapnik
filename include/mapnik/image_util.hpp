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
#include <mapnik/map.hpp>
#include <mapnik/graphics.hpp>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
// stl
#include <string>

namespace mapnik {
    
    class ImageWriterException : public std::exception
    {
    private:
        std::string message_;
    public:
        ImageWriterException(const std::string& message) 
            : message_(message) {}

        ~ImageWriterException() throw() {}

        virtual const char* what() const throw()
        {
            return message_.c_str();
        }
    };

   MAPNIK_DECL void save_to_cairo_file(mapnik::Map const& map,
                                        std::string const& filename,
                                        std::string const& type);

   template <typename T>
   MAPNIK_DECL void save_to_file(T const& image,
                                 std::string const& filename,
                                 std::string const& type);
   // guess type from file extension
   template <typename T>
   MAPNIK_DECL void save_to_file(T const& image,
                                 std::string const& filename);
   
   template <typename T>
   MAPNIK_DECL std::string save_to_string(T const& image,
                                 std::string const& type);

   template <typename T>
   void save_as_png(T const& image,
                    std::string const& filename);
   
   template <typename T>
   void save_as_jpeg(std::string const& filename,
                     int quality,
                     T const& image);
   
   inline bool is_png (std::string const& filename)
   {
      return boost::algorithm::iends_with(filename,std::string(".png"));
   }
   
   inline bool is_jpeg (std::string const& filename)
   {
      return boost::algorithm::iends_with(filename,std::string(".jpg")) ||
         boost::algorithm::iends_with(filename,std::string(".jpeg"));
   }
   
   inline bool is_tiff (std::string const& filename)
   {
      return boost::algorithm::iends_with(filename,std::string(".tif")) ||
         boost::algorithm::iends_with(filename,std::string(".tiff"));
   }

   inline bool is_pdf (std::string const& filename)
   {
      return boost::algorithm::iends_with(filename,std::string(".pdf"));
   }

   inline bool is_svg (std::string const& filename)
   {
      return boost::algorithm::iends_with(filename,std::string(".svg"));
   }

   inline bool is_ps (std::string const& filename)
   {
      return boost::algorithm::iends_with(filename,std::string(".ps"));
   }
      
   inline std::string type_from_filename(std::string const& filename)
   {
      if (is_png(filename)) return "png";
      if (is_jpeg(filename)) return "jpeg";
      if (is_tiff(filename)) return "tiff";
      if (is_pdf(filename)) return "pdf";
      if (is_svg(filename)) return "svg";
      if (is_ps(filename)) return "ps";
      return "unknown";
   }

   inline std::string guess_type( const std::string & filename )
   {
      std::string::size_type idx = filename.find_last_of(".");
      if ( idx != std::string::npos ) {
          return filename.substr( idx + 1 );
      }
      return "<unknown>";
   }
       
   template <typename T>
   double distance(T x0,T y0,T x1,T y1)
   {
      double dx = x1-x0;
      double dy = y1-y0;
      return std::sqrt(dx * dx + dy * dy);
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
   
   template <typename Image>
   inline void scale_image_bilinear (Image& target,const Image& source, double x_off_f=0, double y_off_f=0)
   {

      int source_width=source.width();
      int source_height=source.height();

      int target_width=target.width();
      int target_height=target.height();

      if (source_width<1 || source_height<1 ||
          target_width<1 || target_height<1) return;
      int x=0,y=0,xs=0,ys=0;
      int tw2 = target_width/2;
      int th2 = target_height/2;
      int offs_x = rint((source_width-target_width-x_off_f*2*source_width)/2);
      int offs_y = rint((source_height-target_height-y_off_f*2*source_height)/2);
      unsigned yprt, yprt1, xprt, xprt1;

      //no scaling or subpixel offset
      if (target_height == source_height && target_width == source_width && offs_x == 0 && offs_y == 0){
         for (y=0;y<target_height;++y)
            target.setRow(y,source.getRow(y),target_width);
         return;
      }

      for (y=0;y<target_height;++y)
      {
        ys = (y*source_height+offs_y)/target_height;
        int ys1 = ys+1;
        if (ys1>=source_height)
            ys1--;
        if (ys<0)
            ys=ys1=0;
        if (source_height/2<target_height)
           yprt = (y*source_height+offs_y)%target_height;
        else
           yprt = th2;
        yprt1 = target_height-yprt;
        for (x=0;x<target_width;++x)
        {
            xs = (x*source_width+offs_x)/target_width;
            if (source_width/2<target_width)
               xprt = (x*source_width+offs_x)%target_width;
            else
               xprt = tw2;
            xprt1 = target_width-xprt;
            int xs1 = xs+1;
            if (xs1>=source_width)
                xs1--;
            if (xs<0)
                xs=xs1=0;

            unsigned a = source(xs,ys);
            unsigned b = source(xs1,ys);
            unsigned c = source(xs,ys1);
            unsigned d = source(xs1,ys1);
            unsigned out=0;
            unsigned t = 0;

            for(int i=0; i<4; i++){
                unsigned p,r,s;
                // X axis
                p = a&0xff;
                r = b&0xff;
                if (p!=r)
                    r = (r*xprt+p*xprt1+tw2)/target_width;
                p = c&0xff;
                s = d&0xff;
                if (p!=s)
                    s = (s*xprt+p*xprt1+tw2)/target_width;
                // Y axis
                if (r!=s)
                    r = (s*yprt+r*yprt1+th2)/target_height;
                // channel up
                out |= r << t;
                t += 8;
                a >>= 8;
                b >>= 8;
                c >>= 8;
                d >>= 8;
            }
            target(x,y)=out;
        }
     }
   }

   template <typename Image>
   inline void scale_image_bilinear8 (Image& target,const Image& source, double x_off_f=0, double y_off_f=0)
   {

      int source_width=source.width();
      int source_height=source.height();

      int target_width=target.width();
      int target_height=target.height();

      if (source_width<1 || source_height<1 ||
          target_width<1 || target_height<1) return;
      int x=0,y=0,xs=0,ys=0;
      int tw2 = target_width/2;
      int th2 = target_height/2;
      int offs_x = rint((source_width-target_width-x_off_f*2*source_width)/2);
      int offs_y = rint((source_height-target_height-y_off_f*2*source_height)/2);
      unsigned yprt, yprt1, xprt, xprt1;

      //no scaling or subpixel offset
      if (target_height == source_height && target_width == source_width && offs_x == 0 && offs_y == 0){
         for (y=0;y<target_height;++y)
            target.setRow(y,source.getRow(y),target_width);
         return;
      }

      for (y=0;y<target_height;++y)
      {
        ys = (y*source_height+offs_y)/target_height;
        int ys1 = ys+1;
        if (ys1>=source_height)
            ys1--;
        if (ys<0)
            ys=ys1=0;
        if (source_height/2<target_height)
           yprt = (y*source_height+offs_y)%target_height;
        else
           yprt = th2;
        yprt1 = target_height-yprt;
        for (x=0;x<target_width;++x)
        {
            xs = (x*source_width+offs_x)/target_width;
            if (source_width/2<target_width)
               xprt = (x*source_width+offs_x)%target_width;
            else
               xprt = tw2;
            xprt1 = target_width-xprt;
            int xs1 = xs+1;
            if (xs1>=source_width)
                xs1--;
            if (xs<0)
                xs=xs1=0;

            unsigned a = source(xs,ys);
            unsigned b = source(xs1,ys);
            unsigned c = source(xs,ys1);
            unsigned d = source(xs1,ys1);
            unsigned p,r,s;
            // X axis
            p = a&0xff;
            r = b&0xff;
            if (p!=r)
                r = (r*xprt+p*xprt1+tw2)/target_width;
            p = c&0xff;
            s = d&0xff;
            if (p!=s)
                s = (s*xprt+p*xprt1+tw2)/target_width;
            // Y axis
            if (r!=s)
                r = (s*yprt+r*yprt1+th2)/target_height;
            target(x,y)=(0xff<<24) | (r<<16) | (r<<8) | r;
        }
     }
   }

   inline MAPNIK_DECL void save_to_file (Image32 const& image,
                                         std::string const& file,
                                         std::string const& type) 
   {
      save_to_file<ImageData32>(image.data(),file,type);
   }
   
   inline MAPNIK_DECL void save_to_file(Image32 const& image,
                                        std::string const& file) 
   {
      save_to_file<ImageData32>(image.data(),file);
   }

   inline MAPNIK_DECL std::string save_to_string(Image32 const& image,
                                        std::string const& type)
   {
      return save_to_string<ImageData32>(image.data(),type);
   }
   
#ifdef _MSC_VER
   template MAPNIK_DECL void save_to_file<ImageData32>(ImageData32 const&,
                                                       std::string const&,
                                                       std::string const&);
   template MAPNIK_DECL void save_to_file<ImageData32>(ImageData32 const&,
                                                       std::string const&);
   template MAPNIK_DECL std::string save_to_string<ImageData32>(ImageData32 const&,
                                                       std::string const&);
   
   template MAPNIK_DECL void save_to_file<image_view<ImageData32> > (image_view<ImageData32> const&,
                                                                     std::string const&,
                                                                     std::string const&);
 
   template MAPNIK_DECL void save_to_file<image_view<ImageData32> > (image_view<ImageData32> const&,
                                                                     std::string const&);
   
   template MAPNIK_DECL std::string save_to_string<image_view<ImageData32> > (image_view<ImageData32> const&,
                                                                     std::string const&);
#endif

}

#endif //IMAGE_UTIL_HPP
