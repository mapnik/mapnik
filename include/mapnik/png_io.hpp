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

//$Id$

#include <mapnik/octree.hpp>
extern "C"
{
#include <png.h>
}

namespace mapnik {
   
   template <typename T>
   void write_data (png_structp png_ptr, png_bytep data, png_size_t length)
   {
      T * out = static_cast<T*>(png_get_io_ptr(png_ptr));
      out->write(reinterpret_cast<char*>(data), length);
   }

   template <typename T>
   void flush_data (png_structp png_ptr)
   {
      T * out = static_cast<T*>(png_get_io_ptr(png_ptr));
      out->flush();
   }
   
   template <typename T1, typename T2>
   void save_as_png(T1 & file , T2 const& image)
   {        
      png_voidp error_ptr=0;
      png_structp png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                  error_ptr,0, 0);
       
      if (!png_ptr) return;
      
      // switch on optimization only if supported
#if defined(PNG_LIBPNG_VER) && (PNG_LIBPNG_VER >= 10200) && defined(PNG_MMX_CODE_SUPPORTED)
      png_uint_32 mask, flags;
      flags = png_get_asm_flags(png_ptr);
      mask = png_get_asm_flagmask(PNG_SELECT_READ | PNG_SELECT_WRITE);
      png_set_asm_flags(png_ptr, flags | mask);
#endif
      png_set_filter (png_ptr, 0, PNG_FILTER_NONE);
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr)
      {
         png_destroy_write_struct(&png_ptr,(png_infopp)0);
         return;
      }
      if (setjmp(png_jmpbuf(png_ptr)))
      {
         png_destroy_write_struct(&png_ptr, &info_ptr);
         return;
      }
      png_set_write_fn (png_ptr, &file, &write_data<T1>, &flush_data<T1>);
        
      //png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
      //png_set_compression_strategy(png_ptr, Z_FILTERED);
      png_set_IHDR(png_ptr, info_ptr,image.width(),image.height(),8,
                   PNG_COLOR_TYPE_RGB_ALPHA,PNG_INTERLACE_NONE,
                   PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
      png_write_info(png_ptr, info_ptr);
      for (unsigned i=0;i<image.height();i++)
      {
         png_write_row(png_ptr,(png_bytep)image.getRow(i));
      }

      png_write_end(png_ptr, info_ptr);
      png_destroy_write_struct(&png_ptr, &info_ptr);
   }

   template <typename T>
   void reduce_8  (T const& in, ImageData8 & out, octree<rgb> & tree)
   {
      unsigned width = in.width();
      unsigned height = in.height();
      for (unsigned y = 0; y < height; ++y)
      {
         mapnik::ImageData32::pixel_type const * row = in.getRow(y);
         mapnik::ImageData8::pixel_type  * row_out = out.getRow(y);
         for (unsigned x = 0; x < width; ++x)
         {
            unsigned val = row[x];
            mapnik::rgb c((val)&0xff, (val>>8)&0xff, (val>>16) & 0xff);
            uint8_t index = tree.quantize(c);
            row_out[x] = index;
         }
      }
   }
         
   template <typename T>
   void reduce_4 (T const& in, ImageData8 & out, octree<rgb> & tree)
   {
      unsigned width = in.width();
      unsigned height = in.height();
      
      for (unsigned y = 0; y < height; ++y)
      {
         mapnik::ImageData32::pixel_type const * row = in.getRow(y);
         mapnik::ImageData8::pixel_type  * row_out = out.getRow(y);
         
         for (unsigned x = 0; x < width; ++x)
         {
            unsigned val = row[x];
            mapnik::rgb c((val)&0xff, (val>>8)&0xff, (val>>16) & 0xff);
            uint8_t index = tree.quantize(c);
            if (x%2 >  0) index = index<<4;
            row_out[x>>1] |= index;  
         }
      }
   }
   
   // 1-bit but only one color.
   template <typename T>
   void reduce_1(T const&, ImageData8 & out, octree<rgb> &)
   {
      out.set(0); // only one color!!!
   }  
   
   template <typename T>
   void save_as_png(T & file, std::vector<mapnik::rgb> & palette, 
                    mapnik::ImageData8 const& image, 
                    unsigned width,
                    unsigned height,
                    unsigned color_depth)
   {        
      png_voidp error_ptr=0;
      png_structp png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                  error_ptr,0, 0);
      
      if (!png_ptr) return;
   
      // switch on optimization only if supported
#if defined(PNG_LIBPNG_VER) && (PNG_LIBPNG_VER >= 10200) && defined(PNG_MMX_CODE_SUPPORTED)
      png_uint_32 mask, flags;
      flags = png_get_asm_flags(png_ptr);
      mask = png_get_asm_flagmask(PNG_SELECT_READ | PNG_SELECT_WRITE);
      png_set_asm_flags(png_ptr, flags | mask);
#endif
      png_set_filter (png_ptr, 0, PNG_FILTER_NONE);
      png_infop info_ptr = png_create_info_struct(png_ptr);
      if (!info_ptr)
      {
         png_destroy_write_struct(&png_ptr,(png_infopp)0);
         return;
      }
      if (setjmp(png_jmpbuf(png_ptr)))
      {
         png_destroy_write_struct(&png_ptr, &info_ptr);
         return;
      }
      png_set_write_fn (png_ptr, &file, &write_data<T>, &flush_data<T>);
          
      png_set_IHDR(png_ptr, info_ptr,width,height,color_depth,
                   PNG_COLOR_TYPE_PALETTE,PNG_INTERLACE_NONE,
                   PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);

      png_set_PLTE(png_ptr,info_ptr,reinterpret_cast<png_color*>(&palette[0]),palette.size());
      
      png_write_info(png_ptr, info_ptr);
      for (unsigned i=0;i<height;i++)
      {
         png_write_row(png_ptr,(png_bytep)image.getRow(i));
      }
   
      png_write_end(png_ptr, info_ptr);
      png_destroy_write_struct(&png_ptr, &info_ptr);
   }
   
   template <typename T1,typename T2>
   void save_as_png256(T1 & file, T2 const& image)
   {
      octree<rgb> tree(256);
      unsigned width = image.width();
      unsigned height = image.height();
      for (unsigned y = 0; y < height; ++y)
      {
         typename T2::pixel_type const * row = image.getRow(y);
         for (unsigned x = 0; x < width; ++x)
         {
            unsigned val = row[x];
            tree.insert(mapnik::rgb((val)&0xff, (val>>8)&0xff, (val>>16) & 0xff));
         }
      }
      
      std::vector<rgb> palette;
      tree.create_palette(palette);
      assert(palette.size() <= 256);
      
      if (palette.size() > 16 )
      {
         // >16 && <=256 colors -> write 8-bit color depth
         ImageData8 reduced_image(width,height);   
         reduce_8(image,reduced_image,tree);
         save_as_png(file,palette,reduced_image,width,height,8);         
      }
      else if (palette.size() == 1) 
      {
         // 1 color image ->  write 1-bit color depth PNG
         unsigned image_width  = (int(0.125*width) + 7)&~7;
         unsigned image_height = height;
         ImageData8 reduced_image(image_width,image_height);
         reduce_1(image,reduced_image,tree); 
         save_as_png(file,palette,reduced_image,width,height,1);
      }
      else 
      {
         // <=16 colors -> write 4-bit color depth PNG
         unsigned image_width  = (int(0.5*width) + 3)&~3;
         unsigned image_height = height;
         ImageData8 reduced_image(image_width,image_height);
         reduce_4(image,reduced_image,tree);
         save_as_png(file,palette,reduced_image,width,height,4);
      }      
   }
}
