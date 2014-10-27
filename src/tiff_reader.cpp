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

extern "C"
{
#include <tiffio.h>
}

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#pragma GCC diagnostic pop

// stl
#include <memory>

namespace mapnik { namespace impl {

static toff_t tiff_seek_proc(thandle_t fd, toff_t off, int whence)
{
    std::istream* in = reinterpret_cast<std::istream*>(fd);

    switch(whence)
    {
    case SEEK_SET:
        in->seekg(off, std::ios_base::beg);
        break;
    case SEEK_CUR:
        in->seekg(off, std::ios_base::cur);
        break;
    case SEEK_END:
        in->seekg(off, std::ios_base::end);
        break;
    }
    return static_cast<toff_t>(in->tellg());
}

static int tiff_close_proc(thandle_t)
{
    return 0;
}

static toff_t tiff_size_proc(thandle_t fd)
{
    std::istream* in = reinterpret_cast<std::istream*>(fd);
    std::ios::pos_type pos = in->tellg();
    in->seekg(0, std::ios::end);
    std::ios::pos_type len = in->tellg();
    in->seekg(pos);
    return static_cast<toff_t>(len);
}

static tsize_t tiff_read_proc(thandle_t fd, tdata_t buf, tsize_t size)
{
    std::istream * in = reinterpret_cast<std::istream*>(fd);
    std::streamsize request_size = size;
    if (static_cast<tsize_t>(request_size) != size)
        return static_cast<tsize_t>(-1);
    in->read(reinterpret_cast<char*>(buf), request_size);
    return static_cast<tsize_t>(in->gcount());
}

static tsize_t tiff_write_proc(thandle_t , tdata_t , tsize_t)
{
    return 0;
}

static void tiff_unmap_proc(thandle_t, tdata_t, toff_t)
{
}

static int tiff_map_proc(thandle_t, tdata_t* , toff_t*)
{
    return 0;
}

}

template <typename T>
class tiff_reader : public image_reader
{
    using tiff_ptr = std::shared_ptr<TIFF>;
    using source_type = T;
    using input_stream = boost::iostreams::stream<source_type>;

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
    source_type source_;
    input_stream stream_;
    int read_method_;
    std::size_t width_;
    std::size_t height_;
    int rows_per_strip_;
    int tile_width_;
    int tile_height_;
    tiff_ptr tif_;
    bool premultiplied_alpha_;
    bool has_alpha_;
public:
    enum TiffType {
        generic=1,
        stripped,
        tiled
    };
    explicit tiff_reader(std::string const& file_name);
    tiff_reader(char const* data, std::size_t size);
    virtual ~tiff_reader();
    unsigned width() const;
    unsigned height() const;
    inline bool has_alpha() const { return has_alpha_; }
    bool premultiplied_alpha() const;
    void read(unsigned x,unsigned y,image_data_32& image);
private:
    tiff_reader(const tiff_reader&);
    tiff_reader& operator=(const tiff_reader&);
    void init();
    void read_generic(unsigned x,unsigned y,image_data_32& image);
    void read_stripped(unsigned x,unsigned y,image_data_32& image);
    void read_tiled(unsigned x,unsigned y,image_data_32& image);
    TIFF* open(std::istream & input);
};

namespace
{

image_reader* create_tiff_reader(std::string const& file)
{
    return new tiff_reader<boost::iostreams::file_source>(file);
}

image_reader* create_tiff_reader2(char const * data, std::size_t size)
{
    return new tiff_reader<boost::iostreams::array_source>(data, size);
}

const bool registered = register_image_reader("tiff",create_tiff_reader);
const bool registered2 = register_image_reader("tiff", create_tiff_reader2);

}

template <typename T>
tiff_reader<T>::tiff_reader(std::string const& file_name)
    : source_(file_name, std::ios_base::in | std::ios_base::binary),
      stream_(source_),
      read_method_(generic),
      width_(0),
      height_(0),
      rows_per_strip_(0),
      tile_width_(0),
      tile_height_(0),
      premultiplied_alpha_(false),
      has_alpha_(false)
{
    if (!stream_) throw image_reader_exception("TIFF reader: cannot open file "+ file_name);
    init();
}

template <typename T>
tiff_reader<T>::tiff_reader(char const* data, std::size_t size)
    : source_(data, size),
      stream_(source_),
      read_method_(generic),
      width_(0),
      height_(0),
      rows_per_strip_(0),
      tile_width_(0),
      tile_height_(0),
      premultiplied_alpha_(false),
      has_alpha_(false)
{
    if (!stream_) throw image_reader_exception("TIFF reader: cannot open image stream ");
    stream_.seekg(0, std::ios::beg);
    init();
}

template <typename T>
void tiff_reader<T>::init()
{
    // avoid calling TIFFs global structures
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    TIFF* tif = open(stream_);

    if (!tif) throw image_reader_exception("Can't open tiff file");

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
        uint16 extrasamples = 0;
        uint16* sampleinfo = nullptr;
        if (TIFFGetField(tif, TIFFTAG_EXTRASAMPLES,
                              &extrasamples, &sampleinfo))
        {
            has_alpha_ = true;
            if (extrasamples == 1 &&
                sampleinfo[0] == EXTRASAMPLE_ASSOCALPHA)
            {
                premultiplied_alpha_ = true;
            }
        }
    }
    else
    {
        throw image_reader_exception(msg);
    }
}

template <typename T>
tiff_reader<T>::~tiff_reader()
{
}

template <typename T>
unsigned tiff_reader<T>::width() const
{
    return width_;
}

template <typename T>
unsigned tiff_reader<T>::height() const
{
    return height_;
}

template <typename T>
bool tiff_reader<T>::premultiplied_alpha() const
{
    return premultiplied_alpha_;
}

template <typename T>
void tiff_reader<T>::read(unsigned x,unsigned y,image_data_32& image)
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

template <typename T>
void tiff_reader<T>::read_generic(unsigned, unsigned, image_data_32&)
{
    TIFF* tif = open(stream_);
    if (tif)
    {
        MAPNIK_LOG_DEBUG(tiff_reader) << "tiff_reader: TODO - tiff is not stripped or tiled";
    }
}

template <typename T>
void tiff_reader<T>::read_tiled(unsigned x0,unsigned y0,image_data_32& image)
{
    TIFF* tif = open(stream_);
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
            ty0 = std::max(y0,(unsigned)y) - y;
            ty1 = std::min(height+y0,(unsigned)(y+tile_height_)) - y;

            int n0=tile_height_-ty1;
            int n1=tile_height_-ty0-1;

            for (int x=start_x;x<end_x;x+=tile_width_)
            {

                if (!TIFFReadRGBATile(tif,x,y,buf)) break;

                tx0=std::max(x0,(unsigned)x);
                tx1=std::min(width+x0,(unsigned)(x+tile_width_));
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

template <typename T>
void tiff_reader<T>::read_stripped(unsigned x0,unsigned y0,image_data_32& image)
{
    TIFF* tif = open(stream_);
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
        tx1=std::min(width+x0,(unsigned)width_);

        for (unsigned y=start_y; y < end_y; y+=rows_per_strip_)
        {
            ty0 = std::max(y0,y)-y;
            ty1 = std::min(height+y0,y+rows_per_strip_)-y;

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

template <typename T>
TIFF* tiff_reader<T>::open(std::istream & input)
{
    if (!tif_)
    {
        tif_ = tiff_ptr(TIFFClientOpen("tiff_input_stream", "rm",
                                       reinterpret_cast<thandle_t>(&input),
                                       impl::tiff_read_proc,
                                       impl::tiff_write_proc,
                                       impl::tiff_seek_proc,
                                       impl::tiff_close_proc,
                                       impl::tiff_size_proc,
                                       impl::tiff_map_proc,
                                       impl::tiff_unmap_proc), tiff_closer());
    }
    return tif_.get();
}

} // namespace mapnik
