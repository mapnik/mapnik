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

//$Id: graphics.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
// mapnik
#include <mapnik/color.hpp>
#include <mapnik/gamma.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/envelope.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/global.hpp>

// stl
#include <cmath>
#include <string>
#include <cassert>

// cairo
#ifdef HAVE_CAIRO
#include <cairomm/surface.h>
#endif

namespace mapnik
{

   struct Multiply
   {
      inline static void mergeRGB(unsigned const &r0, unsigned const &g0, unsigned const &b0,
                                  unsigned &r1, unsigned &g1, unsigned &b1)
      {
         r1 = r1*r0/255;
         g1 = g1*g0/255;
         b1 = b1*b0/255;
      }
   };
   struct Multiply2
   {
      inline static void mergeRGB(unsigned const &r0, unsigned const &g0, unsigned const &b0,
                                  unsigned &r1, unsigned &g1, unsigned &b1)
      {
         r1 = r1*r0/128;
         if (r1>255) r1=255;
         g1 = g1*g0/128;
         if (g1>255) g1=255;
         b1 = b1*b0/128;
         if (b1>255) b1=255;
                }
   };
   struct Divide
   {
      inline static void mergeRGB(unsigned const &r0, unsigned const &g0, unsigned const &b0,
                                  unsigned &r1, unsigned &g1, unsigned &b1)
      {
         r1 = r0*256/(r1+1);
         g1 = g0*256/(g1+1);
         b1 = b0*256/(b1+1);
      }
   };
   struct Divide2
   {
      inline static void mergeRGB(unsigned const &r0, unsigned const &g0, unsigned const &b0,
                                  unsigned &r1, unsigned &g1, unsigned &b1)
      {
         r1 = r0*128/(r1+1);
         g1 = g0*128/(g1+1);
         b1 = b0*128/(b1+1);
      }
   };
   struct Screen
        {
                inline static void mergeRGB(unsigned const &r0, unsigned const &g0, unsigned const &b0,
                                            unsigned &r1, unsigned &g1, unsigned &b1)
                {
                      r1 = 255 - (255-r0)*(255-r1)/255;
                      g1 = 255 - (255-g0)*(255-g1)/255;
                      b1 = 255 - (255-b0)*(255-b1)/255;
                }
        };
        struct HardLight
        {
                inline static void mergeRGB(unsigned const &r0, unsigned const &g0, unsigned const &b0,
                                            unsigned &r1, unsigned &g1, unsigned &b1)
                {
                      r1 = (r1>128)?255-(255-r0)*(255-2*(r1-128))/256:r0*r1*2/256;
                      g1 = (g1>128)?255-(255-g0)*(255-2*(g1-128))/256:g0*g1*2/256;
                      b1 = (b1>128)?255-(255-b0)*(255-2*(b1-128))/256:b0*b1*2/256;
                }
        };
        struct MergeGrain
        {
                inline static void mergeRGB(unsigned const &r0, unsigned const &g0, unsigned const &b0,
                                            unsigned &r1, unsigned &g1, unsigned &b1)
                {
                      r1 = (r1+r0>128)?r1+r0-128:0;
                      if (r1>255) r1=255;
                      g1 = (g1+g0>128)?g1+g0-128:0;
                      if (g1>255) g1=255;
                      b1 = (b1+b0>128)?b1+b0-128:0;
                      if (b1>255) b1=255;
                }
        };
        struct MergeGrain2
        {
                inline static void mergeRGB(unsigned const &r0, unsigned const &g0, unsigned const &b0,
                                            unsigned &r1, unsigned &g1, unsigned &b1)
                {
                      r1 = (2*r1+r0>256)?2*r1+r0-256:0;
                      if (r1>255) r1=255;
                      g1 = (2*g1+g0>256)?2*g1+g0-256:0;
                      if (g1>255) g1=255;
                      b1 = (2*b1+b0>256)?2*b1+b0-256:0;
                      if (b1>255) b1=255;
                }
        };

    class MAPNIK_DECL Image32
    {
    private:
        unsigned width_;
        unsigned height_;
        color background_;
        ImageData32 data_;
    public:
        Image32(int width,int height);
        Image32(Image32 const& rhs);
#ifdef HAVE_CAIRO
        Image32(Cairo::RefPtr<Cairo::ImageSurface> rhs);
#endif
        ~Image32();
        void setBackground(color const& background);
        const color& getBackground() const;
        const ImageData32& data() const;

        inline ImageData32& data()
        {
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

        inline image_view<ImageData32> get_view(unsigned x,unsigned y, unsigned w,unsigned h)
        {
            return image_view<ImageData32>(x,y,w,h,data_);
        }

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
        inline void blendPixel(int x,int y,unsigned int rgba1,int t)
        {
            blendPixel2(x,y,rgba1,t,1.0);  // do not change opacity
        }

        inline void blendPixel2(int x,int y,unsigned int rgba1,int t,double opacity)
        {
            if (checkBounds(x,y))
            {
                unsigned rgba0 = data_(x,y);
#ifdef MAPNIK_BIG_ENDIAN
                unsigned a1 = (int)((rgba1 & 0xff) * opacity) & 0xff; // adjust for desired opacity
                a1 = (t*a1) / 255;
                if (a1 == 0) return;
                unsigned r1 = (rgba1 >> 24) & 0xff;
                unsigned g1 = (rgba1 >> 16 ) & 0xff;
                unsigned b1 = (rgba1 >> 8) & 0xff;

                unsigned a0 = (rgba0 & 0xff);
                unsigned r0 = ((rgba0 >> 24 ) & 0xff) * a0;
                unsigned g0 = ((rgba0 >> 16 ) & 0xff) * a0;
                unsigned b0 = ((rgba0 >> 8) & 0xff) * a0;

                a0 = ((a1 + a0) << 8) - a0*a1;

                r0 = ((((r1 << 8) - r0) * a1 + (r0 << 8)) / a0);
                g0 = ((((g1 << 8) - g0) * a1 + (g0 << 8)) / a0);
                b0 = ((((b1 << 8) - b0) * a1 + (b0 << 8)) / a0);
                a0 = a0 >> 8;
                data_(x,y)= (a0)| (b0 << 8) |  (g0 << 16) | (r0 << 24) ;
#else
                unsigned a1 = (int)(((rgba1 >> 24) & 0xff) * opacity) & 0xff; // adjust for desired opacity
                a1 = (t*a1) / 255;
                if (a1 == 0) return;
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
#endif
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
                  unsigned int* row_to =  data_.getRow(y);
                  unsigned int const * row_from = data.getRow(y-y0);

                  for (int x = box.minx(); x < box.maxx(); ++x)
                  {
#ifdef MAPNIK_BIG_ENDIAN
                     row_to[x] = row_from[x-x0];
#else
                     if (row_from[x-x0] & 0xff000000)
                     {
                        row_to[x] = row_from[x-x0];
                     }
#endif
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
                   unsigned int* row_to =  data_.getRow(y);
                   unsigned int const * row_from = data.getRow(y-y0);
                   for (int x = box.minx(); x < box.maxx(); ++x)
                   {
                      unsigned rgba0 = row_to[x];
                      unsigned rgba1 = row_from[x-x0];

#ifdef MAPNIK_BIG_ENDIAN
                      unsigned a1 = rgba1 & 0xff;
                      if (a1 == 0) continue;
                      unsigned r1 = (rgba1 >> 24) & 0xff;
                      unsigned g1 = (rgba1 >> 16 ) & 0xff;
                      unsigned b1 = (rgba1 >> 8) & 0xff;

                      unsigned a0 = rgba0 & 0xff;
                      unsigned r0 = ((rgba0 >> 24) & 0xff) * a0;
                      unsigned g0 = ((rgba0 >> 16 ) & 0xff) * a0;
                      unsigned b0 = ((rgba0 >> 8) & 0xff) * a0;

                      a0 = ((a1 + a0) << 8) - a0*a1;

                      r0 = ((((r1 << 8) - r0) * a1 + (r0 << 8)) / a0);
                      g0 = ((((g1 << 8) - g0) * a1 + (g0 << 8)) / a0);
                      b0 = ((((b1 << 8) - b0) * a1 + (b0 << 8)) / a0);
                      a0 = a0 >> 8;
                      row_to[x] = (a0) | (b0 << 8) |  (g0 << 16) | (r0 << 24) ;
#else
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
                      row_to[x] = (a0 << 24)| (b0 << 16) |  (g0 << 8) | (r0) ;
#endif
                   }
                }
            }
        }

        inline void set_rectangle_alpha2(ImageData32 const& data, unsigned x0, unsigned y0, float opacity)
        {
            Envelope<int> ext0(0,0,width_,height_);
            Envelope<int> ext1(x0,y0,x0 + data.width(),y0 + data.height());

            if (ext0.intersects(ext1))
            {
                Envelope<int> box = ext0.intersect(ext1);
                for (int y = box.miny(); y < box.maxy(); ++y)
                {
                   unsigned int* row_to =  data_.getRow(y);
                   unsigned int const * row_from = data.getRow(y-y0);
                   for (int x = box.minx(); x < box.maxx(); ++x)
                   {
                      unsigned rgba0 = row_to[x];
                      unsigned rgba1 = row_from[x-x0];
#ifdef MAPNIK_BIG_ENDIAN
                      unsigned a1 = int( (rgba1 & 0xff) * opacity );
                      if (a1 == 0) continue;
                      unsigned r1 = (rgba1 >> 24) & 0xff;
                      unsigned g1 = (rgba1 >> 16 ) & 0xff;
                      unsigned b1 = (rgba1 >> 8) & 0xff;

                      unsigned a0 = rgba0 & 0xff;
                      unsigned r0 = (rgba0 >> 24) & 0xff ;
                      unsigned g0 = (rgba0 >> 16 ) & 0xff;
                      unsigned b0 = (rgba0 >> 8) & 0xff;

                      r0 = byte(((r1 - r0) * a1 + (r0 << 8)) >> 8);
                      g0 = byte(((g1 - g0) * a1 + (g0 << 8)) >> 8);
                      b0 = byte(((b1 - b0) * a1 + (b0 << 8)) >> 8);
                      a0 = byte((a1 + a0) - ((a1 * a0 + 255) >> 8));

                      row_to[x] = (a0)| (b0 << 8) |  (g0 << 16) | (r0 << 24) ;
#else
                      unsigned a1 = int( ((rgba1 >> 24) & 0xff) * opacity );
                      if (a1 == 0) continue;
                      unsigned r1 = rgba1 & 0xff;
                      unsigned g1 = (rgba1 >> 8 ) & 0xff;
                      unsigned b1 = (rgba1 >> 16) & 0xff;

                      unsigned a0 = (rgba0 >> 24) & 0xff;
                      unsigned r0 = rgba0 & 0xff ;
                      unsigned g0 = (rgba0 >> 8 ) & 0xff;
                      unsigned b0 = (rgba0 >> 16) & 0xff;

                      r0 = byte(((r1 - r0) * a1 + (r0 << 8)) >> 8);
                      g0 = byte(((g1 - g0) * a1 + (g0 << 8)) >> 8);
                      b0 = byte(((b1 - b0) * a1 + (b0 << 8)) >> 8);
                      a0 = byte((a1 + a0) - ((a1 * a0 + 255) >> 8));
                      
                      row_to[x] = (a0 << 24)| (b0 << 16) |  (g0 << 8) | (r0) ;
#endif
                    }
                }
            }
        }

        template <typename MergeMethod>
        inline void merge_rectangle(ImageData32 const& data, unsigned x0, unsigned y0, float opacity)
        {
            Envelope<int> ext0(0,0,width_,height_);
            Envelope<int> ext1(x0,y0,x0 + data.width(),y0 + data.height());

            if (ext0.intersects(ext1))
            {
                Envelope<int> box = ext0.intersect(ext1);
                for (int y = box.miny(); y < box.maxy(); ++y)
                {
                   unsigned int* row_to =  data_.getRow(y);
                   unsigned int const * row_from = data.getRow(y-y0);
                   for (int x = box.minx(); x < box.maxx(); ++x)
                   {
                      unsigned rgba0 = row_to[x];
                      unsigned rgba1 = row_from[x-x0];
#ifdef MAPNIK_BIG_ENDIAN
                      unsigned a1 = int( (rgba1 & 0xff) * opacity );
                      if (a1 == 0) continue;
                      unsigned r1 = (rgba1 >> 24)& 0xff;
                      unsigned g1 = (rgba1 >> 16 ) & 0xff;
                      unsigned b1 = (rgba1 >> 8) & 0xff;

                      unsigned a0 = rgba0 & 0xff;
                      unsigned r0 = (rgba0 >> 24) & 0xff ;
                      unsigned g0 = (rgba0 >> 16 ) & 0xff;
                      unsigned b0 = (rgba0 >> 8) & 0xff;

                      unsigned a = (a1 * 255 + (255 - a1) * a0 + 127)/255;

                      MergeMethod::mergeRGB(r0,g0,b0,r1,g1,b1);

                      r0 = (r1*a1 + (((255 - a1) * a0 + 127)/255) * r0 + 127)/a;
                      g0 = (g1*a1 + (((255 - a1) * a0 + 127)/255) * g0 + 127)/a;
                      b0 = (b1*a1 + (((255 - a1) * a0 + 127)/255) * b0 + 127)/a;

                      row_to[x] = (a)| (b0 << 8) |  (g0 << 16) | (r0 << 24) ;
#else
                      unsigned a1 = int( ((rgba1 >> 24) & 0xff) * opacity );
                      if (a1 == 0) continue;
                      unsigned r1 = rgba1 & 0xff;
                      unsigned g1 = (rgba1 >> 8 ) & 0xff;
                      unsigned b1 = (rgba1 >> 16) & 0xff;

                      unsigned a0 = (rgba0 >> 24) & 0xff;
                      unsigned r0 = rgba0 & 0xff ;
                      unsigned g0 = (rgba0 >> 8 ) & 0xff;
                      unsigned b0 = (rgba0 >> 16) & 0xff;

                      unsigned a = (a1 * 255 + (255 - a1) * a0 + 127)/255;

                      MergeMethod::mergeRGB(r0,g0,b0,r1,g1,b1);

                      r0 = (r1*a1 + (((255 - a1) * a0 + 127)/255) * r0 + 127)/a;
                      g0 = (g1*a1 + (((255 - a1) * a0 + 127)/255) * g0 + 127)/a;
                      b0 = (b1*a1 + (((255 - a1) * a0 + 127)/255) * b0 + 127)/a;

                      row_to[x] = (a << 24)| (b0 << 16) |  (g0 << 8) | (r0) ;
#endif
                    }
                }
            }
        }
    };
}
#endif //GRAPHICS_HPP
