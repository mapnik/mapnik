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

#include "image_reader.hh"
#include <tiffio.h>
#include <iostream>

namespace mapnik 
{

    class TiffReader : public ImageReader
    {
    private:
        std::string file_name_;
        int read_method_;
        unsigned width_;
        unsigned height_;
        int rows_per_strip_;
        int tile_width_;
        int tile_height_;
    public:
        enum
	{
	    generic=1,
	    stripped,
	    tiled
	};
        explicit TiffReader(const std::string& file_name);
        virtual ~TiffReader();
        unsigned width() const;
        unsigned height() const;
        void read(unsigned x,unsigned y,ImageData32& image);
    private:
        TiffReader(const TiffReader&);
        TiffReader& operator=(const TiffReader&);
        void init();
        void read_generic(unsigned x,unsigned y,ImageData32& image);
        void read_stripped(unsigned x,unsigned y,ImageData32& image);
        void read_tiled(unsigned x,unsigned y,ImageData32& image);
    };

    namespace
    {
	ImageReader* createTiffReader(const std::string& file)
	{
	    return new TiffReader(file);
	}

	const bool registered = 
	    ImageReaderFactory::instance()->register_product("tiff",createTiffReader);
    }

    TiffReader::TiffReader(const std::string& file_name)
	: file_name_(file_name),
	  read_method_(generic),
	  width_(0),
	  height_(0),
	  rows_per_strip_(0),
	  tile_width_(0),
	  tile_height_(0)
    {
	try
	{
	    init();
	}
	catch (ImageReaderException& ex)
	{
	    std::cerr<<ex.what()<<std::endl;
	    throw;
	}
    }


    void TiffReader::init()
    {
	TIFF* tif = TIFFOpen(file_name_.c_str(), "r");
	if (!tif) throw ImageReaderException("cannot open "+file_name_);
	char msg[1024];

	if (TIFFRGBAImageOK(tif,msg))
	{
	    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width_);
	    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height_);
	    if (TIFFIsTiled(tif))
	    {
		TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width_);
		TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height_);
		read_method_=tiled;
	    }
	    else if (TIFFGetField(tif,TIFFTAG_ROWSPERSTRIP,&rows_per_strip_)!=0)
	    {
		read_method_=stripped;
	    }
	    TIFFClose(tif);
	}
	else
	{
	    TIFFClose(tif);
	    throw ImageReaderException(msg);
	}
    }


    TiffReader::~TiffReader()
    {
	//
    }


    unsigned TiffReader::width() const
    {
	return width_;
    }


    unsigned TiffReader::height() const
    {
	return height_;
    }


    void TiffReader::read(unsigned x,unsigned y,ImageData32& image)
    {    
	if (read_method_==stripped)
	{
	    read_stripped(x,y,image);
	}
	else if (read_method_==tiled)
	{
	    read_tiled(x,y,image);
	}
	else
	{
	    read_generic(x,y,image);
	}
    }


    void TiffReader::read_generic(unsigned x,unsigned y,ImageData32& image)
    {
	TIFF* tif = TIFFOpen(file_name_.c_str(), "r");
	if (tif)
	{
	    //todo
	    TIFFClose(tif);
	}
    }


    void TiffReader::read_tiled(unsigned x0,unsigned y0,ImageData32& image)
    {

	TIFF* tif=TIFFOpen(file_name_.c_str(), "r");
	if (tif)
	{
	    uint32* buf = (uint32*)_TIFFmalloc(tile_width_*tile_height_*sizeof(uint32));
	    int width=image.width();
	    int height=image.height();

	    int start_y=(y0/tile_height_)*tile_height_;
	    int end_y=((y0+height)/tile_height_+1)*tile_height_;

	    int start_x=(x0/tile_width_)*tile_width_;
	    int end_x=((x0+width)/tile_width_+1)*tile_width_;

	    int row=0;
	    int tx0,tx1,ty0,ty1;
	    for (int y=start_y;y<end_y;y+=tile_height_)
	    {
		ty0=std::max(y0,(unsigned)y)-y;
		ty1=std::min(height+y0,(unsigned)(y+tile_height_))-y;
		for (int x=start_x;x<end_x;x+=tile_width_)
		{

		    if (!TIFFReadRGBATile(tif,x,y,buf)) break;

		    tx0=std::max(x0,(unsigned)x)-x;
		    tx1=std::min(width+x0,(unsigned)(x+tile_width_))-x;

		    row=y+ty0-y0;
		    for (int n=tile_height_-ty0-1;n>=tile_height_-ty1;--n)
		    {
			image.setRow(row,x+tx0-x0,x+tx1-x0,(const unsigned*)&buf[n*tile_width_+tx0]);
			++row;
		    }
		}
	    }
	    _TIFFfree(buf);
	    TIFFClose(tif);
	}
    }


    void TiffReader::read_stripped(unsigned x,unsigned  y,ImageData32& image)
    {
	TIFF* tif = TIFFOpen(file_name_.c_str(), "r");
	if (tif)
	{
	    uint32* buf = (uint32*)_TIFFmalloc(width_*rows_per_strip_*sizeof(uint32));
	    int width=image.width();
	    int height=image.height();

	    int start=(y/rows_per_strip_)*rows_per_strip_;
	    int end=((y+height)/rows_per_strip_+1)*rows_per_strip_;
	    int extra=y%rows_per_strip_;
	    int j=-extra;
	    int w=std::min(width_,(unsigned)width);//todo should be unsigned
	    for (int row=start; row < end; row+=rows_per_strip_)
	    {
		if (!TIFFReadRGBAStrip(tif,row,buf)) break;
		for (int i=rows_per_strip_-1;i>=0;--i)
		{
		    if (j>=0 && j<height)
		    {
			image.setRow(j,(const unsigned*)&buf[i*width_+x],
				     w*sizeof(uint32));
		    }
		    ++j;
		}
	    }
	    _TIFFfree(buf);
	    TIFFClose(tif);
	}
    }
}

