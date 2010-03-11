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
#include <mapnik/global.hpp>
#include <mapnik/octree.hpp>
#include <mapnik/hextree.hpp>
#include <mapnik/global.hpp>

extern "C"
{
#include <png.h>
}

#define MAX_OCTREE_LEVELS 4

#ifdef MAPNIK_BIG_ENDIAN
  #define U2RED(x) (((x)>>24)&0xff)
  #define U2GREEN(x) (((x)>>16)&0xff)
  #define U2BLUE(x) (((x)>>8)&0xff)
  #define U2ALPHA(x) ((x)&0xff)
#else
  #define U2RED(x) ((x)&0xff)
  #define U2GREEN(x) (((x)>>8)&0xff)
  #define U2BLUE(x) (((x)>>16)&0xff)
  #define U2ALPHA(x) (((x)>>24)&0xff)
#endif

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
   void reduce_8  (T const& in, ImageData8 & out, octree<rgb> trees[], unsigned limits[], unsigned levels, std::vector<unsigned> & alpha)
   {
      unsigned width = in.width();
      unsigned height = in.height();

      //unsigned alphaCount[alpha.size()];
      std::vector<unsigned> alphaCount(alpha.size());
      for(unsigned i=0; i<alpha.size(); i++)
      {
         alpha[i] = 0;
         alphaCount[i] = 0;
      }
      
      for (unsigned y = 0; y < height; ++y)
      {
         mapnik::ImageData32::pixel_type const * row = in.getRow(y);
         mapnik::ImageData8::pixel_type  * row_out = out.getRow(y);
         for (unsigned x = 0; x < width; ++x)
         {
            unsigned val = row[x];
            mapnik::rgb c(U2RED(val), U2GREEN(val), U2BLUE(val));
            byte index = 0;
            int idx = -1;
            for(int j=levels-1; j>0; j--){
               if (U2ALPHA(val)>=limits[j]) {
                  index = idx = trees[j].quantize(c);
                  break;
               }
            }
            if (idx>=0 && idx<(int)alpha.size())
            {
               alpha[idx]+=U2ALPHA(val);
               alphaCount[idx]++;
            }
            row_out[x] = index;
         }
      }
      for(unsigned i=0; i<alpha.size(); i++) 
      {
         if (alphaCount[i]!=0)
            alpha[i] /= alphaCount[i];
      }
   }

   template <typename T>
   void reduce_4 (T const& in, ImageData8 & out, octree<rgb> trees[], unsigned limits[], unsigned levels, std::vector<unsigned> & alpha)
   {
      unsigned width = in.width();
      unsigned height = in.height();

      //unsigned alphaCount[alpha.size()];
      std::vector<unsigned> alphaCount(alpha.size());
      for(unsigned i=0; i<alpha.size(); i++)
      {
         alpha[i] = 0;
         alphaCount[i] = 0;
      }

      for (unsigned y = 0; y < height; ++y)
      {
         mapnik::ImageData32::pixel_type const * row = in.getRow(y);
         mapnik::ImageData8::pixel_type  * row_out = out.getRow(y);

         for (unsigned x = 0; x < width; ++x)
         {
            unsigned val = row[x];
            mapnik::rgb c(U2RED(val), U2GREEN(val), U2BLUE(val));
            byte index = 0;
            int idx=-1;
            for(int j=levels-1; j>0; j--){
               if (U2ALPHA(val)>=limits[j]) {
                  index = idx = trees[j].quantize(c);
                  break;
               }
            }
            if (idx>=0 && idx<(int)alpha.size())
            {
               alpha[idx]+=U2ALPHA(val);
               alphaCount[idx]++;
            }
            if (x%2 == 0) index = index<<4;
            row_out[x>>1] |= index;
         }
      }
      for(unsigned i=0; i<alpha.size(); i++) 
      {
         if (alphaCount[i]!=0)
            alpha[i] /= alphaCount[i];
      }
   }

   // 1-bit but only one color.
   template <typename T>
   void reduce_1(T const&, ImageData8 & out, octree<rgb> trees[], unsigned limits[], std::vector<unsigned> & alpha)
   {
      out.set(0); // only one color!!!
   }

   template <typename T>
   void save_as_png(T & file, std::vector<mapnik::rgb> & palette,
                    mapnik::ImageData8 const& image,
                    unsigned width,
                    unsigned height,
                    unsigned color_depth,
                    std::vector<unsigned> &alpha)
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

      // make transparent lowest indexes, so tRNS is small
      if (alpha.size()>0)
      {
	  std::vector<png_byte> trans(alpha.size());
	  unsigned alphaSize=0;//truncate to nonopaque values
	  for(unsigned i=0; i < alpha.size(); i++) 
	  {
	      trans[i]=alpha[i];
	      if (alpha[i]<255)
		  alphaSize = i+1;
	  }
	  if (alphaSize>0)
	      png_set_tRNS(png_ptr, info_ptr, (png_bytep)&trans[0], alphaSize, 0);
      }

      png_write_info(png_ptr, info_ptr);
      for (unsigned i=0;i<height;i++)
      {
         png_write_row(png_ptr,(png_bytep)image.getRow(i));
      }

      png_write_end(png_ptr, info_ptr);
      png_destroy_write_struct(&png_ptr, &info_ptr);
   }

   template <typename T1,typename T2>
   void save_as_png256(T1 & file, T2 const& image, const unsigned max_colors = 256, int trans_mode = -1)
   {
      // number of alpha ranges in png256 format; 2 results in smallest image with binary transparency
      // 3 is minimum for semitransparency, 4 is recommended, anything else is worse
      const unsigned TRANSPARENCY_LEVELS = (trans_mode==2||trans_mode<0)?MAX_OCTREE_LEVELS:2;
      unsigned width = image.width();
      unsigned height = image.height();
      unsigned alphaHist[256];//transparency histogram
      unsigned semiCount = 0;//sum of semitransparent pixels
      unsigned meanAlpha = 0;
      for(int i=0; i<256; i++){
        alphaHist[i] = 0;
      }
      for (unsigned y = 0; y < height; ++y){
         for (unsigned x = 0; x < width; ++x){
            unsigned val = U2ALPHA((unsigned)image.getRow(y)[x]);
            if (trans_mode==0)
               val=255;
            alphaHist[val]++;
            meanAlpha += val;
            if (val>0 && val<255)
                semiCount++;
         }
      }
      meanAlpha /= width*height;

      // transparency ranges division points
      unsigned limits[MAX_OCTREE_LEVELS+1];
      limits[0] = 0;
      limits[1] = (alphaHist[0]>0)?1:0;
      limits[TRANSPARENCY_LEVELS] = 256;
      unsigned alphaHistSum = 0;
      for(unsigned j=1; j<TRANSPARENCY_LEVELS; j++)
         limits[j] = limits[1];
      for(unsigned i=1; i<256; i++){
          alphaHistSum += alphaHist[i];
          for(unsigned j=1; j<TRANSPARENCY_LEVELS; j++){
              if (alphaHistSum<semiCount*(j)/4)
                limits[j] = i;
          }
      }
      // avoid too wide full transparent range
      if (limits[1]>256/(TRANSPARENCY_LEVELS-1))
         limits[1]=256/(TRANSPARENCY_LEVELS-1);
      // avoid too wide full opaque range
      if (limits[TRANSPARENCY_LEVELS-1]<212)
         limits[TRANSPARENCY_LEVELS-1]=212;
      if (TRANSPARENCY_LEVELS==2) {
         limits[1]=127;
      }
      // estimated number of colors from palette assigned to chosen ranges
      unsigned cols[MAX_OCTREE_LEVELS];
      // count colors
      for(unsigned j=1; j<=TRANSPARENCY_LEVELS; j++) {
         cols[j-1] = 0;
         for(unsigned i=limits[j-1]; i<limits[j]; i++){
            cols[j-1] += alphaHist[i];
         }
      }

      unsigned divCoef = width*height-cols[0];
      if (divCoef==0) divCoef = 1;
      cols[0] = cols[0]>0?1:0; // fully transparent color (one or not at all)

      if (max_colors>=64) {
          // give chance less populated but not empty cols to have at least few colors(12)
          unsigned minCols = (12+1)*divCoef/(max_colors-cols[0]);
          for(unsigned j=1; j<TRANSPARENCY_LEVELS; j++) if (cols[j]>12 && cols[j]<minCols) {
             divCoef += minCols-cols[j];
             cols[j] = minCols;
          }
      }
      unsigned usedColors = cols[0];
      for(unsigned j=1; j<TRANSPARENCY_LEVELS-1; j++){
         cols[j] = cols[j]*(max_colors-cols[0])/divCoef;
         usedColors += cols[j];
      }
      // use rest for most opaque group of pixels
      cols[TRANSPARENCY_LEVELS-1] = max_colors-usedColors;

      //no transparency
      if (trans_mode == 0)
      {
         limits[1] = 0;
         cols[0] = 0;
         cols[1] = max_colors;
      }

      // octree table for separate alpha range with 1-based index (0 is fully transparent: no color)
      octree<rgb> trees[MAX_OCTREE_LEVELS];
      for(unsigned j=1; j<TRANSPARENCY_LEVELS; j++)
         trees[j].setMaxColors(cols[j]);
      for (unsigned y = 0; y < height; ++y)
      {
         typename T2::pixel_type const * row = image.getRow(y);
         for (unsigned x = 0; x < width; ++x)
         {
            unsigned val = row[x];
            
            // insert to proper tree based on alpha range
            for(unsigned j=TRANSPARENCY_LEVELS-1; j>0; j--){
               if (cols[j]>0 && U2ALPHA(val)>=limits[j]) {
                  trees[j].insert(mapnik::rgb(U2RED(val), U2GREEN(val), U2BLUE(val)));
                  break;
               }
            }
         }
      }
      unsigned leftovers = 0;
      std::vector<rgb> palette;
      palette.reserve(max_colors);
      if (cols[0])
         palette.push_back(rgb(0,0,0));

      for(unsigned j=1; j<TRANSPARENCY_LEVELS; j++) {
         if (cols[j]>0) {
            if (leftovers>0) {
               cols[j] += leftovers;
               trees[j].setMaxColors(cols[j]);
               leftovers = 0;
            }
            std::vector<rgb> pal;
            trees[j].setOffset(palette.size());
            trees[j].create_palette(pal);
            assert(pal.size() <= max_colors);
            leftovers = cols[j]-pal.size();
            cols[j] = pal.size();
            for(unsigned i=0; i<pal.size(); i++){
               palette.push_back(pal[i]);
            }
            assert(palette.size() <= 256);
         }
      }

      //transparency values per palette index
      std::vector<unsigned> alphaTable;
      //alphaTable.resize(palette.size());//allow semitransparency also in almost opaque range
      if (trans_mode != 0)
         alphaTable.resize(palette.size() - cols[TRANSPARENCY_LEVELS-1]);
      
      if (palette.size() > 16 )
      {
         // >16 && <=256 colors -> write 8-bit color depth
         ImageData8 reduced_image(width,height);
         reduce_8(image, reduced_image, trees, limits, TRANSPARENCY_LEVELS, alphaTable);
         save_as_png(file,palette,reduced_image,width,height,8,alphaTable);
      }
      else if (palette.size() == 1)
      {
         // 1 color image ->  write 1-bit color depth PNG
         unsigned image_width  = (int(0.125*width) + 7)&~7;
         unsigned image_height = height;
         ImageData8 reduced_image(image_width,image_height);
         reduce_1(image,reduced_image,trees, limits, alphaTable);
         if (meanAlpha<255 && cols[0]==0) {
            alphaTable.resize(1);
            alphaTable[0] = meanAlpha;
         }
         save_as_png(file,palette,reduced_image,width,height,1,alphaTable);
      }
      else
      {
         // <=16 colors -> write 4-bit color depth PNG
         unsigned image_width  = (int(0.5*width) + 3)&~3;
         unsigned image_height = height;
         ImageData8 reduced_image(image_width,image_height);
         reduce_4(image, reduced_image, trees, limits, TRANSPARENCY_LEVELS, alphaTable);
         save_as_png(file,palette,reduced_image,width,height,4,alphaTable);
      }
   }

   template <typename T1,typename T2>
   void save_as_png256_hex(T1 & file, T2 const& image, int colors = 256, int trans_mode = -1, double gamma = 2.0)
   {
      unsigned width = image.width();
      unsigned height = image.height();

      // structure for color quantization
      hextree<mapnik::rgba> tree(colors);
      if (trans_mode >= 0)
         tree.setTransMode(trans_mode);
      if (gamma > 0)
         tree.setGamma(gamma);

      for (unsigned y = 0; y < height; ++y)
      {
         typename T2::pixel_type const * row = image.getRow(y);
         for (unsigned x = 0; x < width; ++x)
         {
            unsigned val = row[x];
            tree.insert(mapnik::rgba(U2RED(val), U2GREEN(val), U2BLUE(val), U2ALPHA(val)));
         }
      }

      //transparency values per palette index
      std::vector<mapnik::rgba> pal;
      tree.create_palette(pal);
      assert(pal.size() <= colors);

      std::vector<mapnik::rgb> palette;
      std::vector<unsigned> alphaTable;
      for(unsigned i=0; i<pal.size(); i++)
      {
          palette.push_back(rgb(pal[i].r, pal[i].g, pal[i].b));
          alphaTable.push_back(pal[i].a);
      }

      if (palette.size() > 16 )
      {
          // >16 && <=256 colors -> write 8-bit color depth
          ImageData8 reduced_image(width, height);

          for (unsigned y = 0; y < height; ++y)
          {
             mapnik::ImageData32::pixel_type const * row = image.getRow(y);
             mapnik::ImageData8::pixel_type  * row_out = reduced_image.getRow(y);

             for (unsigned x = 0; x < width; ++x)
             {
                unsigned val = row[x];
                mapnik::rgba c(U2RED(val), U2GREEN(val), U2BLUE(val), U2ALPHA(val));
                row_out[x] = tree.quantize(c);
             }
          }
          save_as_png(file, palette, reduced_image, width, height, 8, alphaTable);
      }
      else if (palette.size() == 1)
      {
         // 1 color image ->  write 1-bit color depth PNG
         unsigned image_width  = (int(0.125*width) + 7)&~7;
         unsigned image_height = height;
         ImageData8 reduced_image(image_width, image_height);
         reduced_image.set(0);
         save_as_png(file, palette, reduced_image, width, height, 1, alphaTable);
      }
      else
      {
         // <=16 colors -> write 4-bit color depth PNG
         unsigned image_width  = (int(0.5*width) + 3)&~3;
         unsigned image_height = height;
         ImageData8 reduced_image(image_width, image_height);
          for (unsigned y = 0; y < height; ++y)
          {
             mapnik::ImageData32::pixel_type const * row = image.getRow(y);
             mapnik::ImageData8::pixel_type  * row_out = reduced_image.getRow(y);
             byte index = 0;

             for (unsigned x = 0; x < width; ++x)
             {
                unsigned val = row[x];
                mapnik::rgba c(U2RED(val), U2GREEN(val), U2BLUE(val), U2ALPHA(val));
                index = tree.quantize(c);
                if (x%2 == 0) index = index<<4;
                row_out[x>>1] |= index;
             }
          }
         save_as_png(file, palette, reduced_image, width, height, 4, alphaTable);
      }
   }   
}
