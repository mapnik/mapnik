/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/image_reader.hpp>
// boost
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/operations.hpp>

// stl
#include <iostream>

extern "C"
{
#include <tiffio.h>
}

namespace mapnik
{

using std::min;
using std::max;

class tiff_reader : public image_reader
{
    typedef boost::shared_ptr<TIFF> tiff_ptr;
    struct tiff_closer
    {
        void operator() (TIFF * tif)
        {
            if (tif != 0)
            {
                TIFFClose(tif);
            }
        }
    };

private:
    std::string file_name_;
    int read_method_;
    unsigned width_;
    unsigned height_;
    int rows_per_strip_;
    int tile_width_;
    int tile_height_;
    tiff_ptr tif_;
    bool premultiplied_alpha_;
public:
    enum TiffType {
        generic=1,
        stripped,
        tiled
    };
    explicit tiff_reader(std::string const& file_name);
    virtual ~tiff_reader();
    unsigned width() const;
    unsigned height() const;
    bool premultiplied_alpha() const;
    void read(unsigned x,unsigned y,image_data_32& image);
private:
    tiff_reader(const tiff_reader&);
    tiff_reader& operator=(const tiff_reader&);
    void init();
    void read_generic(unsigned x,unsigned y,image_data_32& image);
    void read_stripped(unsigned x,unsigned y,image_data_32& image);
    void read_tiled(unsigned x,unsigned y,image_data_32& image);
    TIFF* load_if_exists(std::string const& filename);
};

namespace
{
image_reader* create_tiff_reader(std::string const& file)
{
    return new tiff_reader(file);
}

const bool registered = register_image_reader("tiff",create_tiff_reader);
}

tiff_reader::tiff_reader(std::string const& file_name)
    : file_name_(file_name),
      read_method_(generic),
      width_(0),
      height_(0),
      rows_per_strip_(0),
      tile_width_(0),
      tile_height_(0),
      premultiplied_alpha_(false)
{
    init();
}


void tiff_reader::init()
{
    // TODO: error handling
    TIFFSetWarningHandler(0);
    TIFF* tif = load_if_exists(file_name_);
    if (!tif) throw image_reader_exception( std::string("Can't load tiff file: '") + file_name_ + "'");

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
        //TIFFTAG_EXTRASAMPLES
        uint16 extrasamples;
        uint16* sampleinfo;
        TIFFGetFieldDefaulted(tif, TIFFTAG_EXTRASAMPLES,
                              &extrasamples, &sampleinfo);
        if (extrasamples == 1 &&
            sampleinfo[0] == EXTRASAMPLE_ASSOCALPHA)
        {
            premultiplied_alpha_ = true;
        }
    }
    else
    {
        throw image_reader_exception(msg);
    }
}


tiff_reader::~tiff_reader()
{
}


unsigned tiff_reader::width() const
{
    return width_;
}


unsigned tiff_reader::height() const
{
    return height_;
}

bool tiff_reader::premultiplied_alpha() const
{
    return premultiplied_alpha_;
}

void tiff_reader::read(unsigned x,unsigned y,image_data_32& image)
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


void tiff_reader::read_generic(unsigned /*x*/,unsigned /*y*/,image_data_32& /*image*/)
{
    TIFF* tif = load_if_exists(file_name_);
    if (tif)
    {
        MAPNIK_LOG_DEBUG(tiff_reader) << "tiff_reader: TODO - tiff is not stripped or tiled";
    }
}


void tiff_reader::read_tiled(unsigned x0,unsigned y0,image_data_32& image)
{
    TIFF* tif = load_if_exists(file_name_);
    if (tif)
    {
        uint32* buf = (uint32*)_TIFFmalloc(tile_width_*tile_height_*sizeof(uint32));
        int width=image.width();
        int height=image.height();

        int start_y=(y0/tile_height_)*tile_height_;
        int end_y=((y0+height)/tile_height_+1)*tile_height_;

        int start_x=(x0/tile_width_)*tile_width_;
        int end_x=((x0+width)/tile_width_+1)*tile_width_;
        int row,tx0,tx1,ty0,ty1;

        for (int y=start_y;y<end_y;y+=tile_height_)
        {
            ty0 = max(y0,(unsigned)y) - y;
            ty1 = min(height+y0,(unsigned)(y+tile_height_)) - y;

            int n0=tile_height_-ty1;
            int n1=tile_height_-ty0-1;

            for (int x=start_x;x<end_x;x+=tile_width_)
            {

                if (!TIFFReadRGBATile(tif,x,y,buf)) break;

                tx0=max(x0,(unsigned)x);
                tx1=min(width+x0,(unsigned)(x+tile_width_));
                row=y+ty0-y0;
                for (int n=n1;n>=n0;--n)
                {
                    image.setRow(row,tx0-x0,tx1-x0,(const unsigned*)&buf[n*tile_width_+tx0-x]);
                    ++row;
                }
            }
        }
        _TIFFfree(buf);
    }
}


void tiff_reader::read_stripped(unsigned x0,unsigned y0,image_data_32& image)
{
    TIFF* tif = load_if_exists(file_name_);
    if (tif)
    {
        uint32* buf = (uint32*)_TIFFmalloc(width_*rows_per_strip_*sizeof(uint32));

        int width=image.width();
        int height=image.height();

        unsigned start_y=(y0/rows_per_strip_)*rows_per_strip_;
        unsigned end_y=((y0+height)/rows_per_strip_+1)*rows_per_strip_;
        bool laststrip=((unsigned)end_y > height_)?true:false;
        int row,tx0,tx1,ty0,ty1;

        tx0=x0;
        tx1=min(width+x0,(unsigned)width_);

        for (unsigned y=start_y; y < end_y; y+=rows_per_strip_)
        {
            ty0 = max(y0,y)-y;
            ty1 = min(height+y0,y+rows_per_strip_)-y;

            if (!TIFFReadRGBAStrip(tif,y,buf)) break;

            row=y+ty0-y0;

            int n0=laststrip ? 0:(rows_per_strip_-ty1);
            int n1=laststrip ? (ty1-ty0-1):(rows_per_strip_-ty0-1);
            for (int n=n1;n>=n0;--n)
            {
                image.setRow(row,tx0-x0,tx1-x0,(const unsigned*)&buf[n*width_+tx0]);
                ++row;
            }
        }
        _TIFFfree(buf);
    }
}

TIFF* tiff_reader::load_if_exists(std::string const& filename)
{
    if (!tif_)
    {
        boost::filesystem::path path(file_name_);
        if (boost::filesystem::is_regular(path)) // exists and regular file
        {
            // File path is a full file path and does exist
            tif_ = tiff_ptr(TIFFOpen(filename.c_str(), "rb"), tiff_closer());
        }
    }
    return tif_.get();
}

} // namespace mapnik
