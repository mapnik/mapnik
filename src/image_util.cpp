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

//$Id: image_util.cpp 36 2005-04-05 14:32:18Z pavlenko $

// mapnik
#include <mapnik/image_util.hpp>

#include <mapnik/graphics.hpp>
#include <mapnik/memory.hpp>
#include <mapnik/image_view.hpp>
// jpeg png
extern "C"
{
#include <png.h>
#include <jpeglib.h>
}

// stl
#include <string>
#include <iostream>
#include <fstream>

namespace mapnik
{    
   template <typename T>
   void save_to_file(std::string const& filename,
                     std::string const& type,
                     T const& image)
   {
      std::ofstream file (filename.c_str(), std::ios::out| std::ios::trunc|std::ios::binary);
      if (file)
      {
         //all this should go into image_writer factory
         if (type=="png")  save_as_png(file,image);
         else if (type=="jpeg") save_as_jpeg(file,85,image);
      } 
   }
  
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
   
   
#define BUFFER_SIZE 4096

   typedef struct  
   {
         struct jpeg_destination_mgr pub;
         std::ostream * out;
         JOCTET * buffer;
   } dest_mgr;
   
   void init_destination( j_compress_ptr cinfo)
   {
      dest_mgr * dest = reinterpret_cast<dest_mgr*>(cinfo->dest);
      dest->buffer = (JOCTET*) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                                                           BUFFER_SIZE * sizeof(JOCTET));
      dest->pub.next_output_byte = dest->buffer;
      dest->pub.free_in_buffer = BUFFER_SIZE;
   }

   boolean empty_output_buffer (j_compress_ptr cinfo)
   {
      dest_mgr * dest = reinterpret_cast<dest_mgr*>(cinfo->dest);
      dest->out->write((char*)dest->buffer, BUFFER_SIZE);
      if (!*(dest->out)) return false;
      dest->pub.next_output_byte = dest->buffer;
      dest->pub.free_in_buffer = BUFFER_SIZE;
      return true;
   }
   
   void term_destination( j_compress_ptr cinfo)
   {
      dest_mgr * dest = reinterpret_cast<dest_mgr*>(cinfo->dest);
      size_t size  = BUFFER_SIZE - dest->pub.free_in_buffer;
      if (size > 0) 
      {
         dest->out->write((char*)dest->buffer, size);
      }
      dest->out->flush();
   }
   
   template <typename T1, typename T2>
   void save_as_jpeg(T1 & file,int quality, T2 const& image)
   {  
      struct jpeg_compress_struct cinfo;
      struct jpeg_error_mgr jerr;

      int width=image.width();
      int height=image.height();
	
      cinfo.err = jpeg_std_error(&jerr);
      jpeg_create_compress(&cinfo);
    
      cinfo.dest = (struct jpeg_destination_mgr *)(*cinfo.mem->alloc_small)
         ((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(dest_mgr));
      dest_mgr * dest = (dest_mgr*) cinfo.dest;
      dest->pub.init_destination = init_destination;
      dest->pub.empty_output_buffer = empty_output_buffer;
      dest->pub.term_destination = term_destination;
      dest->out = &file;
      
      //jpeg_stdio_dest(&cinfo, fp);
      cinfo.image_width = width;
      cinfo.image_height = height;
      cinfo.input_components = 3;
      cinfo.in_color_space = JCS_RGB; 
      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, quality,1);
      jpeg_start_compress(&cinfo, 1);
      JSAMPROW row_pointer[1];
      JSAMPLE* row=reinterpret_cast<JSAMPLE*>( ::operator new (sizeof(JSAMPLE) * width*3));
      while (cinfo.next_scanline < cinfo.image_height) 
      {
         const unsigned* imageRow=image.getRow(cinfo.next_scanline);
         int index=0;
         for (int i=0;i<width;++i)
         {
            row[index++]=(imageRow[i])&0xff;
            row[index++]=(imageRow[i]>>8)&0xff;
            row[index++]=(imageRow[i]>>16)&0xff;
         }
         row_pointer[0] = &row[0];
         (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
      }
      ::operator delete(row);
      
      jpeg_finish_compress(&cinfo);
      jpeg_destroy_compress(&cinfo);
   }  
   
   template void save_to_file<ImageData32>(std::string const&,
                                           std::string const&, 
                                           ImageData32 const&);

   template void save_to_file<image_view<ImageData32> > (std::string const&,
                                                         std::string const&, 
                                                         image_view<ImageData32> const&);
    
}
