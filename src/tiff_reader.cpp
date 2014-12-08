/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
    unsigned bps_;
    unsigned photometric_;
    unsigned bands_;

public:
    enum TiffType {
        generic=1,
        stripped,
        tiled
    };
    explicit tiff_reader(std::string const& file_name);
    tiff_reader(char const* data, std::size_t size);
    virtual ~tiff_reader();
    unsigned width() const final;
    unsigned height() const final;
    inline bool has_alpha() const final { return has_alpha_; }
    bool premultiplied_alpha() const final;
    void read(unsigned x,unsigned y,image_data_rgba8& image) final;
    image_data_any read(unsigned x, unsigned y, unsigned width, unsigned height) final;
private:
    tiff_reader(const tiff_reader&);
    tiff_reader& operator=(const tiff_reader&);
    void init();
    void read_generic(unsigned x,unsigned y,image_data_rgba8& image);
    void read_stripped(unsigned x,unsigned y,image_data_rgba8& image);

    template <typename ImageData>
    void read_tiled(unsigned x,unsigned y, ImageData & image);

    template <typename ImageData>
    image_data_any read_any_gray(unsigned x, unsigned y, unsigned width, unsigned height);

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
      has_alpha_(false),
      bps_(0),
      photometric_(0),
      bands_(1)
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
      has_alpha_(false),
      bps_(0),
      photometric_(0),
      bands_(1)
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

    TIFFGetField(tif,TIFFTAG_BITSPERSAMPLE,&bps_);
    TIFFGetField(tif,TIFFTAG_PHOTOMETRIC,&photometric_);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &bands_);

    MAPNIK_LOG_DEBUG(tiff_reader) << "bits per sample: " << bps_;
    MAPNIK_LOG_DEBUG(tiff_reader) << "photometric: " << photometric_;
    MAPNIK_LOG_DEBUG(tiff_reader) << "bands: " << bands_;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width_);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height_);

    std::uint16_t orientation;
    if (TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation) == 0)
    {
        orientation = 1;
    }
    MAPNIK_LOG_DEBUG(tiff_reader) << "orientation: " << orientation;

    char msg[1024];
    if (true)//TIFFRGBAImageOK(tif,msg))
    {
        if (TIFFIsTiled(tif))
        {
            TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width_);
            TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height_);
            MAPNIK_LOG_DEBUG(tiff_reader) << "reading tiled tiff";
            read_method_ = tiled;
        }
        else if (TIFFGetField(tif,TIFFTAG_ROWSPERSTRIP,&rows_per_strip_)!=0)
        {
            MAPNIK_LOG_DEBUG(tiff_reader) << "reading striped tiff";
            read_method_ = stripped;
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
        MAPNIK_LOG_ERROR(tiff) << msg;
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
void tiff_reader<T>::read(unsigned x,unsigned y,image_data_rgba8& image)
{
    if (read_method_==stripped)
    {
        read_stripped(x,y,image);
    }
    else if (read_method_==tiled)
    {
        read_tiled<image_data_rgba8>(x,y,image);
    }
    else
    {
        read_generic(x,y,image);
    }
}

template <typename T>
template <typename ImageData>
image_data_any tiff_reader<T>::read_any_gray(unsigned x0, unsigned y0, unsigned width, unsigned height)
{

    using image_data_type = ImageData;
    using pixel_type = typename image_data_type::pixel_type;
    if (read_method_ == tiled)
    {
        image_data_type data(width,height);
        read_tiled<image_data_type>(x0, y0, data);
        return image_data_any(std::move(data));
    }
    else
    {
        TIFF* tif = open(stream_);
        if (tif)
        {
            image_data_type data(width, height);
            std::size_t block_size = rows_per_strip_ > 0 ? rows_per_strip_ : tile_height_ ;
            std::ptrdiff_t start_y = y0 - y0 % block_size;
            std::ptrdiff_t end_y = std::min(y0 + height, static_cast<unsigned>(height_));
            std::ptrdiff_t start_x = x0;
            std::ptrdiff_t end_x = std::min(x0 + width, static_cast<unsigned>(width_));
            std::size_t element_size = sizeof(pixel_type);
            std::size_t size_to_allocate = (TIFFScanlineSize(tif) + element_size - 1)/element_size;
            const std::unique_ptr<pixel_type[]> scanline(new pixel_type[size_to_allocate]);
            for  (std::size_t y = start_y; y < end_y; ++y)
            {
                if (-1 != TIFFReadScanline(tif, scanline.get(), y) && (y >= y0))
                {
                    pixel_type * row = data.getRow(y - y0);
                    std::transform(scanline.get() + start_x, scanline.get() + end_x, row, [](pixel_type const& p) { return p;});
                }
            }
            return image_data_any(std::move(data));
        }
    }
    return image_data_any();
}


namespace detail {

struct rgb8
{
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
};

struct rgb8_to_rgba8
{
    std::uint32_t operator() (rgb8 const& in) const
    {
        return ((255 << 24) | (in.r) | (in.g << 8) | (in.b << 16));
    }
};

template <typename T>
struct tiff_reader_traits
{
    using image_data_type = T;
    using pixel_type = typename image_data_type::pixel_type;
    static bool read_tile(TIFF * tif, unsigned x, unsigned y, pixel_type* buf, std::size_t tile_size)
    {
        return (TIFFReadTile(tif, buf, x, y, 0, 0) != -1);
        //return (TIFFReadEncodedTile(tif, TIFFComputeTile(tif, x,y,0,0), buf, TIFFTileSize(tif)) != -1);
    }
};

// specialization for RGB images - TODO: move allocation out to avoid allocating rgb buffer per tile
template <>
struct tiff_reader_traits<image_data_rgba8>
{
    using pixel_type = std::uint32_t;
    static bool read_tile(TIFF * tif, unsigned x, unsigned y, pixel_type* buf, std::size_t tile_size)
    {
        std::unique_ptr<rgb8[]> rgb_buf(new rgb8[tile_size]);
        if (TIFFReadTile(tif, rgb_buf.get(), x, y, 0, 0) != -1)
        {
            std::transform(rgb_buf.get(), rgb_buf.get() + tile_size, buf, detail::rgb8_to_rgba8());
            return true;
        }
        return false;
    }
};

}

template <typename T>
image_data_any tiff_reader<T>::read(unsigned x0, unsigned y0, unsigned width, unsigned height)
{
    switch (photometric_)
    {
    case PHOTOMETRIC_MINISBLACK:
    {
        switch (bps_)
        {
        case 8:
        {
            return read_any_gray<image_data_gray8>(x0, y0, width, height);
        }
        case 16:
        {
            return read_any_gray<image_data_gray16>(x0, y0, width, height);
        }
        case 32:
        {
            return read_any_gray<image_data_gray32f>(x0, y0, width, height);
        }
        }
    }
// read PHOTOMETRIC_RGB expand using RGBA interface
/*
    case  PHOTOMETRIC_RGB:
    {
        switch (bps_)
        {
        case 8:
        {
            TIFF* tif = open(stream_);
            if (tif)
            {
                image_data_rgba8 data(width, height);
                std::size_t element_size = sizeof(detail::rgb8);
                std::size_t size_to_allocate = (TIFFScanlineSize(tif) + element_size - 1)/element_size;
                const std::unique_ptr<detail::rgb8[]> scanline(new detail::rgb8[size_to_allocate]);
                std::ptrdiff_t start_y = y0 - y0 % rows_per_strip_;
                std::ptrdiff_t end_y = std::min(y0 + height, static_cast<unsigned>(height_));
                std::ptrdiff_t start_x = x0;
                std::ptrdiff_t end_x = std::min(x0 + width, static_cast<unsigned>(width_));
                for  (std::size_t y = start_y; y < end_y; ++y)
                {
                    if (-1 != TIFFReadScanline(tif, scanline.get(), y))
                    {
                        if (y >= y0)
                        {
                            image_data_rgba8::pixel_type * row = data.getRow(y - y0);
                            std::transform(scanline.get() + start_x, scanline.get() + end_x, row, detail::rgb8_to_rgba8());
                        }
                    }
                }
                return image_data_any(std::move(data));
            }
            return image_data_any();
        }
        case 16:
        {
            image_data_rgba8 data(width,height);
            read(x0, y0, data);
            return image_data_any(std::move(data));
        }
        case 32:
        {
            image_data_rgba8 data(width,height);
            read(x0, y0, data);
            return image_data_any(std::move(data));
        }
        }
    }
*/
    default:
    {
        //PHOTOMETRIC_PALETTE = 3;
        //PHOTOMETRIC_MASK = 4;
        //PHOTOMETRIC_SEPARATED = 5;
        //PHOTOMETRIC_YCBCR = 6;
        //PHOTOMETRIC_CIELAB = 8;
        //PHOTOMETRIC_ICCLAB = 9;
        //PHOTOMETRIC_ITULAB = 10;
        //PHOTOMETRIC_LOGL = 32844;
        //PHOTOMETRIC_LOGLUV = 32845;
        image_data_rgba8 data(width,height);
        read(x0, y0, data);
        return image_data_any(std::move(data));
    }
    }
    return image_data_any();
}

template <typename T>
void tiff_reader<T>::read_generic(unsigned, unsigned, image_data_rgba8& image)
{
    TIFF* tif = open(stream_);
    if (tif)
    {
        MAPNIK_LOG_ERROR(tiff_reader) << "tiff_reader: TODO - tiff is not stripped or tiled";
    }
}

template <typename T>
template <typename ImageData>
void tiff_reader<T>::read_tiled(unsigned x0,unsigned y0, ImageData & image)
{
    using pixel_type = typename detail::tiff_reader_traits<ImageData>::pixel_type;

    TIFF* tif = open(stream_);
    if (tif)
    {
        std::unique_ptr<pixel_type[]> buf(new pixel_type[tile_width_*tile_height_]);
        int width = image.width();
        int height = image.height();

        int start_y=(y0 / tile_height_) * tile_height_;
        int end_y=((y0 + height) / tile_height_ + 1) * tile_height_;

        int start_x=(x0 / tile_width_) * tile_width_;
        int end_x=((x0 + width) / tile_width_ + 1) * tile_width_;
        int row, tx0, tx1, ty0, ty1;

        for (int y = start_y; y < end_y; y += tile_height_)
        {
            ty0 = std::max(y0, static_cast<unsigned>(y)) - y;
            ty1 = std::min(height + y0, static_cast<unsigned>(y + tile_height_)) - y;

            int n0 = ty0;
            int n1 = ty1;

            for (int x = start_x; x < end_x; x += tile_width_)
            {
                if (!detail::tiff_reader_traits<ImageData>::read_tile(tif, x, y, buf.get(), tile_width_ * tile_height_))
                {
                    MAPNIK_LOG_ERROR(tiff_reader) << "read_tile(...) failed";
                    break;
                }
                tx0 = std::max(x0, static_cast<unsigned>(x));
                tx1 = std::min(width + x0, static_cast<unsigned>(x + tile_width_));
                row = y + ty0 - y0;

                for (int n = n0; n < n1; ++n, ++row)
                {
                    image.setRow(row, tx0 - x0, tx1 - x0, &buf[n * tile_width_ + tx0 - x]);
                }
            }
        }
    }
}


template <typename T>
void tiff_reader<T>::read_stripped(unsigned x0,unsigned y0,image_data_rgba8& image)
{
    TIFF* tif = open(stream_);
    if (tif)
    {
        std::unique_ptr<uint32[]> buf(new uint32_t[width_*rows_per_strip_]);
        int width=image.width();
        int height=image.height();

        unsigned start_y=(y0/rows_per_strip_)*rows_per_strip_;
        unsigned end_y=((y0+height)/rows_per_strip_+1)*rows_per_strip_;
        bool laststrip=(static_cast<unsigned>(end_y) > height_)?true:false;
        int row,tx0,tx1,ty0,ty1;

        tx0=x0;
        tx1=std::min(width+x0,static_cast<unsigned>(width_));

        for (unsigned y=start_y; y < end_y; y+=rows_per_strip_)
        {
            ty0 = std::max(y0,y)-y;
            ty1 = std::min(height+y0,y+rows_per_strip_)-y;

            if (!TIFFReadRGBAStrip(tif,y,buf.get()))
            {
                MAPNIK_LOG_ERROR(tiff_reader) << "TIFFReadRGBAStrip failed";
                break;
            }
            row=y+ty0-y0;

            int n0=laststrip ? 0:(rows_per_strip_-ty1);
            int n1=laststrip ? (ty1-ty0-1):(rows_per_strip_-ty0-1);
            for (int n=n1;n>=n0;--n)
            {
                image.setRow(row,tx0-x0,tx1-x0,static_cast<const unsigned*>(&buf[n*width_+tx0]));
                ++row;
            }
        }
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
