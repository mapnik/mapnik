/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#pragma GCC diagnostic pop

// stl
#include <memory>

namespace mapnik { namespace impl {

static toff_t tiff_seek_proc(thandle_t handle, toff_t off, int whence)
{
    std::istream* in = reinterpret_cast<std::istream*>(handle);

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

static toff_t tiff_size_proc(thandle_t handle)
{
    std::istream* in = reinterpret_cast<std::istream*>(handle);
    std::ios::pos_type pos = in->tellg();
    in->seekg(0, std::ios::end);
    std::ios::pos_type len = in->tellg();
    in->seekg(pos);
    return static_cast<toff_t>(len);
}

static tsize_t tiff_read_proc(thandle_t handle, tdata_t buf, tsize_t size)
{
    std::istream * in = reinterpret_cast<std::istream*>(handle);
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
    tiff_ptr tif_;
    int read_method_;
    int rows_per_strip_;
    int tile_width_;
    int tile_height_;
    std::size_t width_;
    std::size_t height_;
    boost::optional<box2d<double> > bbox_;
    unsigned bps_;
    unsigned sample_format_;
    unsigned photometric_;
    unsigned bands_;
    unsigned planar_config_;
    unsigned compression_;
    bool has_alpha_;
    bool is_tiled_;

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
    boost::optional<box2d<double> > bounding_box() const final;
    inline bool has_alpha() const final { return has_alpha_; }
    void read(unsigned x,unsigned y,image_rgba8& image) final;
    image_any read(unsigned x, unsigned y, unsigned width, unsigned height) final;
    // methods specific to tiff reader
    unsigned bits_per_sample() const { return bps_; }
    unsigned sample_format() const { return sample_format_; }
    unsigned photometric() const { return photometric_; }
    bool is_tiled() const { return is_tiled_; }
    unsigned tile_width() const { return tile_width_; }
    unsigned tile_height() const { return tile_height_; }
    unsigned rows_per_strip() const { return rows_per_strip_; }
    unsigned planar_config() const { return planar_config_; }
    unsigned compression() const { return compression_; }
private:
    tiff_reader(const tiff_reader&);
    tiff_reader& operator=(const tiff_reader&);
    void init();
    void read_generic(std::size_t x,std::size_t y,image_rgba8& image);
    void read_stripped(std::size_t x,std::size_t y,image_rgba8& image);

    template <typename ImageData>
    void read_tiled(std::size_t x,std::size_t y, ImageData & image);

    template <typename ImageData>
    image_any read_any_gray(std::size_t x, std::size_t y, std::size_t width, std::size_t height);

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
      tif_(nullptr),
      read_method_(generic),
      rows_per_strip_(0),
      tile_width_(0),
      tile_height_(0),
      width_(0),
      height_(0),
      bps_(0),
      sample_format_(SAMPLEFORMAT_UINT),
      photometric_(0),
      bands_(1),
      planar_config_(PLANARCONFIG_CONTIG),
      compression_(COMPRESSION_NONE),
      has_alpha_(false),
      is_tiled_(false)
{
    if (!stream_) throw image_reader_exception("TIFF reader: cannot open file "+ file_name);
    init();
}

template <typename T>
tiff_reader<T>::tiff_reader(char const* data, std::size_t size)
    : source_(data, size),
      stream_(source_),
      tif_(nullptr),
      read_method_(generic),
      rows_per_strip_(0),
      tile_width_(0),
      tile_height_(0),
      width_(0),
      height_(0),
      bps_(0),
      sample_format_(SAMPLEFORMAT_UINT),
      photometric_(0),
      bands_(1),
      planar_config_(PLANARCONFIG_CONTIG),
      compression_(COMPRESSION_NONE),
      has_alpha_(false),
      is_tiled_(false)
{
    if (!stream_) throw image_reader_exception("TIFF reader: cannot open image stream ");
    stream_.rdbuf()->pubsetbuf(0, 0);
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
    TIFFGetField(tif,TIFFTAG_SAMPLEFORMAT,&sample_format_);
    TIFFGetField(tif,TIFFTAG_PHOTOMETRIC,&photometric_);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &bands_);

    MAPNIK_LOG_DEBUG(tiff_reader) << "bits per sample: " << bps_;
    MAPNIK_LOG_DEBUG(tiff_reader) << "sample format: " << sample_format_;
    MAPNIK_LOG_DEBUG(tiff_reader) << "photometric: " << photometric_;
    MAPNIK_LOG_DEBUG(tiff_reader) << "bands: " << bands_;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width_);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height_);

    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planar_config_);
    TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression_ );
    TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rows_per_strip_);

    std::uint16_t orientation;
    if (TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation) == 0)
    {
        orientation = 1;
    }
    MAPNIK_LOG_DEBUG(tiff_reader) << "orientation: " << orientation;

    is_tiled_ = TIFFIsTiled(tif);

    if (is_tiled_)
    {
        TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width_);
        TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height_);
        MAPNIK_LOG_DEBUG(tiff_reader) << "tiff is tiled";
        read_method_ = tiled;
    }
    else if (TIFFGetField(tif,TIFFTAG_ROWSPERSTRIP,&rows_per_strip_)!=0)
    {
        MAPNIK_LOG_DEBUG(tiff_reader) << "tiff is stripped";
        read_method_ = stripped;
    }
    //TIFFTAG_EXTRASAMPLES
    uint16 extrasamples = 0;
    uint16* sampleinfo = nullptr;
    if (TIFFGetField(tif, TIFFTAG_EXTRASAMPLES,
                     &extrasamples, &sampleinfo))
    {
        has_alpha_ = true;
        if (extrasamples > 0 &&
            sampleinfo[0] == EXTRASAMPLE_UNSPECIFIED)
        {
            throw std::runtime_error("Unspecified provided for extra samples to tiff reader.");
        }
    }
    // Try extracting bounding box from geoTIFF tags
    {
        uint16 count = 0;
        double *pixelscale;
        double *tilepoint;
        if (TIFFGetField(tif, 33550, &count, &pixelscale) == 1 && count == 3
            && TIFFGetField(tif, 33922 , &count,  &tilepoint) == 1 && count == 6)
        {
            MAPNIK_LOG_DEBUG(tiff_reader) << "PixelScale:" << pixelscale[0] << "," << pixelscale[1] << "," <<  pixelscale[2];
            MAPNIK_LOG_DEBUG(tiff_reader) << "TilePoint:" << tilepoint[0] << "," << tilepoint[1] << "," << tilepoint[2];
            MAPNIK_LOG_DEBUG(tiff_reader) << "          " << tilepoint[3] << "," << tilepoint[4] << "," << tilepoint[5];

            // assuming upper-left
            double lox = tilepoint[3];
            double loy = tilepoint[4];
            double hix = lox + pixelscale[0] * width_;
            double hiy = loy - pixelscale[1] * height_;
            bbox_.reset(box2d<double>(lox, loy, hix, hiy));
            MAPNIK_LOG_DEBUG(tiff_reader) << "Bounding Box:" << *bbox_;
        }

    }
    if (!is_tiled_ &&
        compression_ == COMPRESSION_NONE &&
        planar_config_ == PLANARCONFIG_CONTIG)
    {
        if (height_ > 128 * 1024 * 1024)
        {
            std::size_t line_size = (bands_ * width_ * bps_ + 7) / 8;
            std::size_t default_strip_height = 8192 / line_size;
            if (default_strip_height == 0) default_strip_height = 1;
            std::size_t num_strips = height_ / default_strip_height;
            if (num_strips > 128 * 1024 * 1024)
            {
                throw image_reader_exception("Can't allocate tiff");
            }
        }
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
boost::optional<box2d<double> > tiff_reader<T>::bounding_box() const
{
    return bbox_;
}

template <typename T>
void tiff_reader<T>::read(unsigned x,unsigned y,image_rgba8& image)
{
    if (read_method_==stripped)
    {
        read_stripped(static_cast<std::size_t>(x),static_cast<std::size_t>(y),image);
    }
    else if (read_method_==tiled)
    {
        read_tiled(static_cast<std::size_t>(x),static_cast<std::size_t>(y),image);
    }
    else
    {
        read_generic(static_cast<std::size_t>(x),static_cast<std::size_t>(y),image);
    }
}

template <typename T>
template <typename ImageData>
image_any tiff_reader<T>::read_any_gray(std::size_t x0, std::size_t y0, std::size_t width, std::size_t height)
{
    using image_type = ImageData;
    using pixel_type = typename image_type::pixel_type;
    if (read_method_ == tiled)
    {
        image_type data(width,height);
        read_tiled<image_type>(x0, y0, data);
        return image_any(std::move(data));
    }
    else
    {
        TIFF* tif = open(stream_);
        if (tif)
        {
            image_type data(width, height);
            std::size_t block_size = rows_per_strip_ > 0 ? rows_per_strip_ : tile_height_ ;
            std::size_t start_y = y0 - y0 % block_size;
            std::size_t end_y = std::min(y0 + height, height_);
            std::size_t start_x = x0;
            std::size_t end_x = std::min(x0 + width, width_);
            std::size_t element_size = sizeof(pixel_type);
            std::size_t size_to_allocate = (TIFFScanlineSize(tif) + element_size - 1)/element_size;
            const std::unique_ptr<pixel_type[]> scanline(new pixel_type[size_to_allocate]);
            for  (std::size_t y = start_y; y < end_y; ++y)
            {
                if (-1 != TIFFReadScanline(tif, scanline.get(), y) && (y >= y0))
                {
                    pixel_type * row = data.get_row(y - y0);
                    std::transform(scanline.get() + start_x, scanline.get() + end_x, row, [](pixel_type const& p) { return p;});
                }
            }
            return image_any(std::move(data));
        }
    }
    return image_any();
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
    using image_type = T;
    using pixel_type = typename image_type::pixel_type;
    static bool read_tile(TIFF * tif, std::size_t x, std::size_t y, pixel_type* buf, std::size_t tile_width, std::size_t tile_height)
    {
        return (TIFFReadEncodedTile(tif, TIFFComputeTile(tif, x,y,0,0), buf, tile_width * tile_height * sizeof(pixel_type)) != -1);
    }
};

// default specialization that expands into RGBA
template <>
struct tiff_reader_traits<image_rgba8>
{
    using pixel_type = std::uint32_t;
    static bool read_tile(TIFF * tif, std::size_t x0, std::size_t y0, pixel_type* buf, std::size_t tile_width, std::size_t tile_height)
    {
        if (TIFFReadRGBATile(tif, x0, y0, buf) != -1)
        {
            for (std::size_t y = 0; y < tile_height/2; ++y)
            {
                std::swap_ranges(buf + y * tile_width, buf + (y + 1) * tile_width, buf + (tile_height - y - 1) * tile_width);
            }
            return true;
        }
        return false;
    }
};

}

template <typename T>
image_any tiff_reader<T>::read(unsigned x, unsigned y, unsigned width, unsigned height)
{
    if (width > 10000 || height > 10000)
    {
        throw image_reader_exception("Can't allocate tiff > 10000x10000");
    }
    std::size_t x0 = static_cast<std::size_t>(x);
    std::size_t y0 = static_cast<std::size_t>(y);
    switch (photometric_)
    {
    case PHOTOMETRIC_MINISBLACK:
    case PHOTOMETRIC_MINISWHITE:
    {
        switch (bps_)
        {
        case 8:
        {
            switch (sample_format_)
            {
            case SAMPLEFORMAT_UINT:
            {
                return read_any_gray<image_gray8>(x0, y0, width, height);
            }
            case SAMPLEFORMAT_INT:
            {
                return read_any_gray<image_gray8s>(x0, y0, width, height);
            }
            default:
            {
                throw std::runtime_error("tiff_reader: This sample format is not supported for this bits per sample");
            }
            }
        }
        case 16:
        {
            switch (sample_format_)
            {
            case SAMPLEFORMAT_UINT:
            {
                return read_any_gray<image_gray16>(x0, y0, width, height);
            }
            case SAMPLEFORMAT_INT:
            {
                return read_any_gray<image_gray16s>(x0, y0, width, height);
            }
            default:
            {
                throw std::runtime_error("tiff_reader: This sample format is not supported for this bits per sample");
            }
            }
        }
        case 32:
        {
            switch (sample_format_)
            {
            case SAMPLEFORMAT_UINT:
            {
                return read_any_gray<image_gray32>(x0, y0, width, height);
            }
            case SAMPLEFORMAT_INT:
            {
                return read_any_gray<image_gray32s>(x0, y0, width, height);
            }
            case SAMPLEFORMAT_IEEEFP:
            {
                return read_any_gray<image_gray32f>(x0, y0, width, height);
            }
            default:
            {
                throw std::runtime_error("tiff_reader: This sample format is not supported for this bits per sample");
            }
            }
        }
        case 64:
        {
            switch (sample_format_)
            {
            case SAMPLEFORMAT_UINT:
            {
                return read_any_gray<image_gray64>(x0, y0, width, height);
            }
            case SAMPLEFORMAT_INT:
            {
                return read_any_gray<image_gray64s>(x0, y0, width, height);
            }
            case SAMPLEFORMAT_IEEEFP:
            {
                return read_any_gray<image_gray64f>(x0, y0, width, height);
            }
            default:
            {
                throw std::runtime_error("tiff_reader: This sample format is not supported for this bits per sample");
            }
            }
        }
        }
    }
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
        image_rgba8 data(width,height, true, true);
        read(x0, y0, data);
        return image_any(std::move(data));
    }
    }
    return image_any();
}

template <typename T>
void tiff_reader<T>::read_generic(std::size_t, std::size_t, image_rgba8& image)
{
    TIFF* tif = open(stream_);
    if (tif)
    {
        throw std::runtime_error("tiff_reader: TODO - tiff is not stripped or tiled");
    }
}

template <typename T>
template <typename ImageData>
void tiff_reader<T>::read_tiled(std::size_t x0,std::size_t y0, ImageData & image)
{
    using pixel_type = typename detail::tiff_reader_traits<ImageData>::pixel_type;

    TIFF* tif = open(stream_);
    if (tif)
    {
        std::unique_ptr<pixel_type[]> buf(new pixel_type[tile_width_*tile_height_]);
        std::size_t width = image.width();
        std::size_t height = image.height();
        std::size_t start_y = (y0 / tile_height_) * tile_height_;
        std::size_t end_y = ((y0 + height) / tile_height_ + 1) * tile_height_;
        std::size_t start_x = (x0 / tile_width_) * tile_width_;
        std::size_t end_x = ((x0 + width) / tile_width_ + 1) * tile_width_;
        end_y = std::min(end_y, height_);
        end_x = std::min(end_x, width_);

        for (std::size_t y = start_y; y < end_y; y += tile_height_)
        {
            std::size_t ty0 = std::max(y0, y) - y;
            std::size_t ty1 = std::min(height + y0, y + tile_height_) - y;

            for (std::size_t x = start_x; x < end_x; x += tile_width_)
            {
                if (!detail::tiff_reader_traits<ImageData>::read_tile(tif, x, y, buf.get(), tile_width_, tile_height_))
                {
                    MAPNIK_LOG_DEBUG(tiff_reader) <<  "read_tile(...) failed at " << x << "/" << y << " for " << width_ << "/" << height_ << "\n";
                    break;
                }
                std::size_t tx0 = std::max(x0, x);
                std::size_t tx1 = std::min(width + x0, x + tile_width_);
                std::size_t row = y + ty0 - y0;
                for (std::size_t ty = ty0; ty < ty1; ++ty, ++row)
                {
                    image.set_row(row, tx0 - x0, tx1 - x0, &buf[ty * tile_width_ + tx0 - x]);
                }
            }
        }
    }
}


template <typename T>
void tiff_reader<T>::read_stripped(std::size_t x0,std::size_t y0,image_rgba8& image)
{
    TIFF* tif = open(stream_);
    if (tif)
    {
        image_rgba8 strip(width_,rows_per_strip_,false);
        std::size_t width=image.width();
        std::size_t height=image.height();

        std::size_t start_y=(y0/rows_per_strip_)*rows_per_strip_;
        std::size_t end_y=std::min(y0+height, height_);
        std::size_t tx0,tx1,ty0,ty1;

        tx0=x0;
        tx1=std::min(width+x0,width_);
        std::size_t row = 0;
        for (std::size_t y=start_y; y < end_y; y+=rows_per_strip_)
        {
            ty0 = std::max(y0,y)-y;
            ty1 = std::min(end_y,y+rows_per_strip_)-y;

            if (!TIFFReadRGBAStrip(tif,y,strip.data()))
            {
                MAPNIK_LOG_DEBUG(tiff_reader) << "TIFFReadRGBAStrip failed at " << y << " for " << width_ << "/" << height_ << "\n";
                break;
            }
            // This is in reverse because the TIFFReadRGBAStrip reads inverted
            for (std::size_t ty = ty1; ty > ty0; --ty)
            {
                image.set_row(row,tx0-x0,tx1-x0,&strip.data()[(ty-1)*width_+tx0]);
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
        tif_ = tiff_ptr(TIFFClientOpen("tiff_input_stream", "rcm",
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
