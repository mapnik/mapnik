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

#include <string>
#include <png.h>
#include <jpeglib.h>
#include "graphics.hh"
#include "image_util.hh"
#include "memory.hh"

namespace mapnik
{

    //use memory manager for mem allocation in libpng

    png_voidp malloc_fn(png_structp png_ptr,png_size_t size)
    {
        return Object::operator new(size);
    }
    void free_fn(png_structp png_ptr, png_voidp ptr)
    {
        Object::operator delete(ptr);
    }
    //
    void ImageUtils::save_to_file(const std::string& filename,const std::string& type,const Image32& image)
    {
	//all that should go into image_writer factory
        if (type=="png")
        {
            save_as_png(filename,image);
        } 
	else if (type=="jpeg")
	{
	    save_as_jpeg(filename,85,image);
	}
    }

    void ImageUtils::save_as_png(const std::string& filename,const Image32& image)
    {
        FILE *fp=fopen(filename.c_str(), "wb");
        if (!fp) return;
        png_voidp mem_ptr=0;
        png_structp png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,
						    (png_voidp)mem_ptr,0, 0);
	
        if (!png_ptr) return;
        png_set_mem_fn(png_ptr,mem_ptr,malloc_fn,free_fn);

        // switch on optimization
	//#if defined(PNG_LIBPNG_VER) && (PNG_LIBPNG_VER >= 10200)
        //png_uint_32 mask, flags;

        //flags = png_get_asm_flags(png_ptr);
        //mask = png_get_asm_flagmask(PNG_SELECT_READ | PNG_SELECT_WRITE);
        //png_set_asm_flags(png_ptr, flags | mask);
	//#endif
	png_set_filter (png_ptr, 0, PNG_FILTER_NONE);
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
            png_destroy_write_struct(&png_ptr,(png_infopp)0);
            fclose(fp);
            return;
        }
        if (setjmp(png_jmpbuf(png_ptr)))
        {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            fclose(fp);
            return;
        }

        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr,image.width(),image.height(),8,
            PNG_COLOR_TYPE_RGB_ALPHA,PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);

        const ImageData32& imageData=image.data();

        for (int i=0;i<image.height();i++)
        {
            png_write_row(png_ptr,(png_bytep)imageData.getRow(i));
        }

        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
    }

    void ImageUtils::save_as_jpeg(const std::string& filename,int quality, const Image32& image)
    {
	FILE *fp=fopen(filename.c_str(), "wb");
        if (!fp) return;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	int width=image.width();
	int height=image.height();
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB; 
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality,1);
	jpeg_start_compress(&cinfo, 1);
	JSAMPROW row_pointer[1];
	JSAMPLE* row=new JSAMPLE[width*3];
	const ImageData32& imageData=image.data();
	while (cinfo.next_scanline < cinfo.image_height) 
	{
	    const unsigned* imageRow=imageData.getRow(cinfo.next_scanline);
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
	delete [] row;
	jpeg_finish_compress(&cinfo);
	fclose(fp);
	jpeg_destroy_compress(&cinfo);
    }
}
