#pragma once
// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/util/char_array_buffer.hpp>
extern "C" {
#include <tiffio.h>
}

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
MAPNIK_DISABLE_WARNING_POP
#include <mapnik/mapped_memory_cache.hpp>
#endif
#include "tiff_reader.hpp"

// stl
#include <memory>
#include <fstream>
#include <algorithm>

namespace mapnik {
namespace detail {

MAPNIK_DECL toff_t tiff_seek_proc(thandle_t handle, toff_t off, int whence);
MAPNIK_DECL int tiff_close_proc(thandle_t);
MAPNIK_DECL toff_t tiff_size_proc(thandle_t handle);
MAPNIK_DECL tsize_t tiff_read_proc(thandle_t handle, tdata_t buf, tsize_t size);
MAPNIK_DECL tsize_t tiff_write_proc(thandle_t, tdata_t, tsize_t);
MAPNIK_DECL void tiff_unmap_proc(thandle_t, tdata_t, toff_t);
MAPNIK_DECL int tiff_map_proc(thandle_t, tdata_t*, toff_t*);

template<typename T>
struct tiff_io_traits
{
    using input_stream_type = std::istream;
};

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
template<>
struct tiff_io_traits<boost::interprocess::ibufferstream>
{
    using input_stream_type = boost::interprocess::ibufferstream;
};
#endif
} // namespace detail

template<typename T>
class tiff_reader : public image_reader
{
    using tiff_ptr = std::shared_ptr<TIFF>;
    using source_type = T;
    using input_stream = typename detail::tiff_io_traits<source_type>::input_stream_type;
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    mapnik::mapped_region_ptr mapped_region_;
#endif

    struct tiff_closer
    {
        void operator()(TIFF* tif)
        {
            if (tif != 0)
                TIFFClose(tif);
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
    boost::optional<box2d<double>> bbox_;
    unsigned bps_;
    unsigned sample_format_;
    unsigned photometric_;
    unsigned bands_;
    unsigned planar_config_;
    unsigned compression_;
    bool has_alpha_;
    bool is_tiled_;

  public:
    enum TiffType { generic = 1, stripped, tiled };
    explicit tiff_reader(std::string const& filename);
    tiff_reader(char const* data, std::size_t size);
    virtual ~tiff_reader();
    unsigned width() const final;
    unsigned height() const final;
    boost::optional<box2d<double>> bounding_box() const final;
    inline bool has_alpha() const final
    {
        return has_alpha_;
    }
    void read(unsigned x, unsigned y, image_rgba8& image) final;
    image_any read(unsigned x, unsigned y, unsigned width, unsigned height) final;
    // methods specific to tiff reader
    unsigned bits_per_sample() const
    {
        return bps_;
    }
    unsigned sample_format() const
    {
        return sample_format_;
    }
    unsigned photometric() const
    {
        return photometric_;
    }
    bool is_tiled() const
    {
        return is_tiled_;
    }
    unsigned tile_width() const
    {
        return tile_width_;
    }
    unsigned tile_height() const
    {
        return tile_height_;
    }
    unsigned rows_per_strip() const
    {
        return rows_per_strip_;
    }
    unsigned planar_config() const
    {
        return planar_config_;
    }
    unsigned compression() const
    {
        return compression_;
    }

  private:
    tiff_reader(const tiff_reader&);
    tiff_reader& operator=(const tiff_reader&);
    void init();

    template<typename ImageData>
    void read_generic(std::size_t x, std::size_t y, ImageData& image);

    template<typename ImageData>
    void read_stripped(std::size_t x, std::size_t y, ImageData& image);

    template<typename ImageData>
    void read_tiled(std::size_t x, std::size_t y, ImageData& image);

    template<typename ImageData>
    image_any read_any_gray(std::size_t x, std::size_t y, std::size_t width, std::size_t height);

    TIFF* open(std::istream& input);
};

template<typename T>
tiff_reader<T>::tiff_reader(std::string const& filename)
    :
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    stream_()
    ,
#else
    source_()
    , stream_(&source_)
    ,
#endif

    tif_(nullptr)
    , read_method_(generic)
    , rows_per_strip_(0)
    , tile_width_(0)
    , tile_height_(0)
    , width_(0)
    , height_(0)
    , bps_(0)
    , sample_format_(SAMPLEFORMAT_UINT)
    , photometric_(0)
    , bands_(1)
    , planar_config_(PLANARCONFIG_CONTIG)
    , compression_(COMPRESSION_NONE)
    , has_alpha_(false)
    , is_tiled_(false)
{
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    boost::optional<mapnik::mapped_region_ptr> memory = mapnik::mapped_memory_cache::instance().find(filename, true);

    if (memory)
    {
        mapped_region_ = *memory;
        stream_.buffer(static_cast<char*>(mapped_region_->get_address()), mapped_region_->get_size());
    }
    else
    {
        throw image_reader_exception("could not create file mapping for " + filename);
    }
#else
    source_.open(filename, std::ios_base::in | std::ios_base::binary);
#endif
    if (!stream_)
        throw image_reader_exception("TIFF reader: cannot open file " + filename);
    init();
}

template<typename T>
tiff_reader<T>::tiff_reader(char const* data, std::size_t size)
    : source_(data, size)
    , stream_(&source_)
    , tif_(nullptr)
    , read_method_(generic)
    , rows_per_strip_(0)
    , tile_width_(0)
    , tile_height_(0)
    , width_(0)
    , height_(0)
    , bps_(0)
    , sample_format_(SAMPLEFORMAT_UINT)
    , photometric_(0)
    , bands_(1)
    , planar_config_(PLANARCONFIG_CONTIG)
    , compression_(COMPRESSION_NONE)
    , has_alpha_(false)
    , is_tiled_(false)
{
    if (!stream_)
        throw image_reader_exception("TIFF reader: cannot open image stream ");
    init();
}

template<typename T>
void tiff_reader<T>::init()
{
    // avoid calling TIFFs global structures
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    TIFF* tif = open(stream_);

    if (!tif)
        throw image_reader_exception("Can't open tiff file");

    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps_);
    TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sample_format_);
    TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric_);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &bands_);

    MAPNIK_LOG_DEBUG(tiff_reader) << "bits per sample: " << bps_;
    MAPNIK_LOG_DEBUG(tiff_reader) << "sample format: " << sample_format_;
    MAPNIK_LOG_DEBUG(tiff_reader) << "photometric: " << photometric_;
    MAPNIK_LOG_DEBUG(tiff_reader) << "bands: " << bands_;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width_);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height_);

    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planar_config_);
    TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression_);

    std::uint16_t orientation;
    if (TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation) == 0)
    {
        orientation = 1;
    }
    MAPNIK_LOG_DEBUG(tiff_reader) << "orientation: " << orientation;
    MAPNIK_LOG_DEBUG(tiff_reader) << "planar-config: " << planar_config_;
    is_tiled_ = TIFFIsTiled(tif);

    if (is_tiled_)
    {
        TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width_);
        TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height_);
        MAPNIK_LOG_DEBUG(tiff_reader) << "tiff is tiled";
        read_method_ = tiled;
    }
    else if (TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rows_per_strip_) != 0)
    {
        MAPNIK_LOG_DEBUG(tiff_reader) << "tiff is stripped";
        read_method_ = stripped;
    }
    // TIFFTAG_EXTRASAMPLES
    std::uint16_t extrasamples = 0;
    std::uint16_t* sampleinfo = nullptr;
    if (TIFFGetField(tif, TIFFTAG_EXTRASAMPLES, &extrasamples, &sampleinfo))
    {
        has_alpha_ = true;
        if (extrasamples > 0 && sampleinfo[0] == EXTRASAMPLE_UNSPECIFIED)
        {
            throw image_reader_exception("Unspecified provided for extra samples to tiff reader.");
        }
    }
    // Try extracting bounding box from geoTIFF tags
    {
        std::uint16_t count = 0;
        double* pixelscale;
        double* tilepoint;
        if (TIFFGetField(tif, 33550, &count, &pixelscale) == 1 && count == 3 &&
            TIFFGetField(tif, 33922, &count, &tilepoint) == 1 && count == 6)
        {
            MAPNIK_LOG_DEBUG(tiff_reader)
              << "PixelScale:" << pixelscale[0] << "," << pixelscale[1] << "," << pixelscale[2];
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
    if (!is_tiled_ && compression_ == COMPRESSION_NONE && planar_config_ == PLANARCONFIG_CONTIG)
    {
        if (height_ > 128 * 1024 * 1024)
        {
            std::size_t line_size = (bands_ * width_ * bps_ + 7) / 8;
            std::size_t default_strip_height = 8192 / line_size;
            if (default_strip_height == 0)
                default_strip_height = 1;
            std::size_t num_strips = height_ / default_strip_height;
            if (num_strips > 128 * 1024 * 1024)
            {
                throw image_reader_exception("Can't allocate tiff");
            }
        }
    }
}

template<typename T>
tiff_reader<T>::~tiff_reader()
{}

template<typename T>
unsigned tiff_reader<T>::width() const
{
    return width_;
}

template<typename T>
unsigned tiff_reader<T>::height() const
{
    return height_;
}

template<typename T>
boost::optional<box2d<double>> tiff_reader<T>::bounding_box() const
{
    return bbox_;
}

template<typename T>
void tiff_reader<T>::read(unsigned x, unsigned y, image_rgba8& image)
{
    if (read_method_ == stripped)
    {
        read_stripped(static_cast<std::size_t>(x), static_cast<std::size_t>(y), image);
    }
    else if (read_method_ == tiled)
    {
        read_tiled(static_cast<std::size_t>(x), static_cast<std::size_t>(y), image);
    }
    else
    {
        read_generic(static_cast<std::size_t>(x), static_cast<std::size_t>(y), image);
    }
}

template<typename T>
template<typename ImageData>
image_any tiff_reader<T>::read_any_gray(std::size_t x0, std::size_t y0, std::size_t width, std::size_t height)
{
    using image_type = ImageData;
    using pixel_type = typename image_type::pixel_type;
    if (read_method_ == tiled)
    {
        image_type data(width, height);
        read_tiled<image_type>(x0, y0, data);
        return image_any(std::move(data));
    }
    else if (read_method_ == stripped)
    {
        image_type data(width, height);
        read_stripped<image_type>(x0, y0, data);
        return image_any(std::move(data));
    }
    else
    {
        TIFF* tif = open(stream_);
        if (tif)
        {
            image_type data(width, height);
            std::size_t block_size = rows_per_strip_ > 0 ? rows_per_strip_ : tile_height_;
            std::size_t start_y = y0 - y0 % block_size;
            std::size_t end_y = std::min(y0 + height, height_);
            std::size_t start_x = x0;
            std::size_t end_x = std::min(x0 + width, width_);
            std::size_t element_size = sizeof(pixel_type);
            MAPNIK_LOG_DEBUG(tiff_reader) << "SCANLINE SIZE=" << TIFFScanlineSize(tif);
            std::size_t size_to_allocate = (TIFFScanlineSize(tif) + element_size - 1) / element_size;
            std::unique_ptr<pixel_type[]> const scanline(new pixel_type[size_to_allocate]);
            if (planar_config_ == PLANARCONFIG_CONTIG)
            {
                for (std::size_t y = start_y; y < end_y; ++y)
                {
                    // we have to read all scanlines sequentially from start_y
                    // to be able to use scanline interface with compressed blocks.
                    if (-1 != TIFFReadScanline(tif, scanline.get(), y) && (y >= y0))
                    {
                        pixel_type* row = data.get_row(y - y0);
                        if (bands_ == 1)
                        {
                            std::transform(scanline.get() + start_x,
                                           scanline.get() + end_x,
                                           row,
                                           [](pixel_type const& p) { return p; });
                        }
                        else if (size_to_allocate == bands_ * width_)
                        {
                            // bands_ > 1 => packed bands in grayscale image e.g an extra alpha channel.
                            // Just pick first one for now.
                            pixel_type* buf = scanline.get() + start_x * bands_;
                            std::size_t x_index = 0;
                            for (std::size_t j = 0; j < end_x * bands_; ++j)
                            {
                                if (x_index >= width)
                                    break;
                                if (j % bands_ == 0)
                                {
                                    row[x_index++] = buf[j];
                                }
                            }
                        }
                    }
                }
            }
            else if (planar_config_ == PLANARCONFIG_SEPARATE)
            {
                for (std::size_t s = 0; s < bands_; ++s)
                {
                    for (std::size_t y = start_y; y < end_y; ++y)
                    {
                        if (-1 != TIFFReadScanline(tif, scanline.get(), y) && (y >= y0))
                        {
                            pixel_type* row = data.get_row(y - y0);
                            std::transform(scanline.get() + start_x,
                                           scanline.get() + end_x,
                                           row,
                                           [](pixel_type const& p) { return p; });
                        }
                    }
                }
            }
            return image_any(std::move(data));
        }
    }
    return image_any();
}

template<typename T>
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
        case PHOTOMETRIC_MINISWHITE: {
            switch (bps_)
            {
                case 8: {
                    switch (sample_format_)
                    {
                        case SAMPLEFORMAT_UINT: {
                            return read_any_gray<image_gray8>(x0, y0, width, height);
                        }
                        case SAMPLEFORMAT_INT: {
                            return read_any_gray<image_gray8s>(x0, y0, width, height);
                        }
                        default: {
                            throw image_reader_exception(
                              "tiff_reader: This sample format is not supported for this bits per sample");
                        }
                    }
                }
                case 16: {
                    switch (sample_format_)
                    {
                        case SAMPLEFORMAT_UINT: {
                            return read_any_gray<image_gray16>(x0, y0, width, height);
                        }
                        case SAMPLEFORMAT_INT: {
                            return read_any_gray<image_gray16s>(x0, y0, width, height);
                        }
                        default: {
                            throw image_reader_exception(
                              "tiff_reader: This sample format is not supported for this bits per sample");
                        }
                    }
                }
                case 32: {
                    switch (sample_format_)
                    {
                        case SAMPLEFORMAT_UINT: {
                            return read_any_gray<image_gray32>(x0, y0, width, height);
                        }
                        case SAMPLEFORMAT_INT: {
                            return read_any_gray<image_gray32s>(x0, y0, width, height);
                        }
                        case SAMPLEFORMAT_IEEEFP: {
                            return read_any_gray<image_gray32f>(x0, y0, width, height);
                        }
                        default: {
                            throw image_reader_exception(
                              "tiff_reader: This sample format is not supported for this bits per sample");
                        }
                    }
                }
                case 64: {
                    switch (sample_format_)
                    {
                        case SAMPLEFORMAT_UINT: {
                            return read_any_gray<image_gray64>(x0, y0, width, height);
                        }
                        case SAMPLEFORMAT_INT: {
                            return read_any_gray<image_gray64s>(x0, y0, width, height);
                        }
                        case SAMPLEFORMAT_IEEEFP: {
                            return read_any_gray<image_gray64f>(x0, y0, width, height);
                        }
                        default: {
                            throw image_reader_exception(
                              "tiff_reader: This sample format is not supported for this bits per sample");
                        }
                    }
                }
            }
        }
        default: {
            // PHOTOMETRIC_PALETTE = 3;
            // PHOTOMETRIC_MASK = 4;
            // PHOTOMETRIC_SEPARATED = 5;
            // PHOTOMETRIC_YCBCR = 6;
            // PHOTOMETRIC_CIELAB = 8;
            // PHOTOMETRIC_ICCLAB = 9;
            // PHOTOMETRIC_ITULAB = 10;
            // PHOTOMETRIC_LOGL = 32844;
            // PHOTOMETRIC_LOGLUV = 32845;
            image_rgba8 data(width, height, true, true);
            read(x0, y0, data);
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
    std::uint32_t operator()(rgb8 const& in) const { return ((255 << 24) | (in.r) | (in.g << 8) | (in.b << 16)); }
};

template<typename T>
struct tiff_reader_traits
{
    using image_type = T;
    using pixel_type = typename image_type::pixel_type;

    constexpr static bool reverse = false;
    static bool read_tile(TIFF* tif,
                          std::size_t x,
                          std::size_t y,
                          pixel_type* buf,
                          std::size_t tile_width,
                          std::size_t tile_height)
    {
        std::uint32_t tile_size = TIFFTileSize(tif);
        return (TIFFReadEncodedTile(tif, TIFFComputeTile(tif, x, y, 0, 0), buf, tile_size) != -1);
    }

    static bool
      read_strip(TIFF* tif, std::size_t y, std::size_t rows_per_strip, std::size_t strip_width, pixel_type* buf)
    {
        return (TIFFReadEncodedStrip(tif, y / rows_per_strip, buf, -1) != -1);
    }
};

// default specialization that expands into RGBA
template<>
struct tiff_reader_traits<image_rgba8>
{
    using image_type = image_rgba8;
    using pixel_type = std::uint32_t;
    constexpr static bool reverse = true;
    static bool read_tile(TIFF* tif,
                          std::size_t x0,
                          std::size_t y0,
                          pixel_type* buf,
                          std::size_t tile_width,
                          std::size_t tile_height)
    {
        return (TIFFReadRGBATile(tif, x0, y0, buf) != 0);
    }

    static bool
      read_strip(TIFF* tif, std::size_t y, std::size_t rows_per_strip, std::size_t strip_width, pixel_type* buf)
    {
        return (TIFFReadRGBAStrip(tif, y, buf) != 0);
    }
};

} // namespace detail

template<typename T>
template<typename ImageData>
void tiff_reader<T>::read_generic(std::size_t, std::size_t, ImageData&)
{
    throw image_reader_exception("tiff_reader: TODO - tiff is not stripped or tiled");
}

template<typename T>
template<typename ImageData>
void tiff_reader<T>::read_tiled(std::size_t x0, std::size_t y0, ImageData& image)
{
    using pixel_type = typename detail::tiff_reader_traits<ImageData>::pixel_type;

    TIFF* tif = open(stream_);
    if (tif)
    {
        std::uint32_t tile_size = TIFFTileSize(tif);
        std::unique_ptr<pixel_type[]> tile(new pixel_type[tile_size]);
        std::size_t width = image.width();
        std::size_t height = image.height();
        std::size_t start_y = (y0 / tile_height_) * tile_height_;
        std::size_t end_y = ((y0 + height) / tile_height_ + 1) * tile_height_;
        std::size_t start_x = (x0 / tile_width_) * tile_width_;
        std::size_t end_x = ((x0 + width) / tile_width_ + 1) * tile_width_;
        end_y = std::min(end_y, height_);
        end_x = std::min(end_x, width_);
        bool pick_first_band =
          (bands_ > 1) && (tile_size / (tile_width_ * tile_height_ * sizeof(pixel_type)) == bands_);
        for (std::size_t y = start_y; y < end_y; y += tile_height_)
        {
            std::size_t ty0 = std::max(y0, y) - y;
            std::size_t ty1 = std::min(height + y0, y + tile_height_) - y;

            for (std::size_t x = start_x; x < end_x; x += tile_width_)
            {
                if (!detail::tiff_reader_traits<ImageData>::read_tile(tif, x, y, tile.get(), tile_width_, tile_height_))
                {
                    MAPNIK_LOG_DEBUG(tiff_reader)
                      << "read_tile(...) failed at " << x << "/" << y << " for " << width_ << "/" << height_ << "\n";
                    break;
                }
                if (pick_first_band)
                {
                    std::uint32_t size = tile_width_ * tile_height_ * sizeof(pixel_type);
                    for (std::uint32_t n = 0; n < size; ++n)
                    {
                        tile[n] = tile[n * bands_];
                    }
                }
                std::size_t tx0 = std::max(x0, x);
                std::size_t tx1 = std::min(width + x0, x + tile_width_);
                std::size_t row_index = y + ty0 - y0;

                if (detail::tiff_reader_traits<ImageData>::reverse)
                {
                    for (std::size_t ty = ty0; ty < ty1; ++ty, ++row_index)
                    {
                        // This is in reverse because the TIFFReadRGBATile reads are inverted
                        image.set_row(row_index,
                                      tx0 - x0,
                                      tx1 - x0,
                                      &tile[(tile_height_ - ty - 1) * tile_width_ + tx0 - x]);
                    }
                }
                else
                {
                    for (std::size_t ty = ty0; ty < ty1; ++ty, ++row_index)
                    {
                        image.set_row(row_index, tx0 - x0, tx1 - x0, &tile[ty * tile_width_ + tx0 - x]);
                    }
                }
            }
        }
    }
}

template<typename T>
template<typename ImageData>
void tiff_reader<T>::read_stripped(std::size_t x0, std::size_t y0, ImageData& image)
{
    using pixel_type = typename detail::tiff_reader_traits<ImageData>::pixel_type;
    TIFF* tif = open(stream_);
    if (tif)
    {
        std::uint32_t strip_size = TIFFStripSize(tif);
        std::unique_ptr<pixel_type[]> strip(new pixel_type[strip_size]);
        std::size_t width = image.width();
        std::size_t height = image.height();

        std::size_t start_y = (y0 / rows_per_strip_) * rows_per_strip_;
        std::size_t end_y = std::min(y0 + height, height_);
        std::size_t tx0, tx1, ty0, ty1;
        tx0 = x0;
        tx1 = std::min(width + x0, width_);
        std::size_t row = 0;
        bool pick_first_band = (bands_ > 1) && (strip_size / (width_ * rows_per_strip_ * sizeof(pixel_type)) == bands_);
        for (std::size_t y = start_y; y < end_y; y += rows_per_strip_)
        {
            ty0 = std::max(y0, y) - y;
            ty1 = std::min(end_y, y + rows_per_strip_) - y;

            if (!detail::tiff_reader_traits<ImageData>::read_strip(tif, y, rows_per_strip_, width_, strip.get()))
            {
                MAPNIK_LOG_DEBUG(tiff_reader)
                  << "TIFFRead(Encoded|RGBA)Strip failed at " << y << " for " << width_ << "/" << height_ << "\n";
                break;
            }
            if (pick_first_band)
            {
                std::uint32_t size = width_ * rows_per_strip_ * sizeof(pixel_type);
                for (std::uint32_t n = 0; n < size; ++n)
                {
                    strip[n] = strip[bands_ * n];
                }
            }

            if (detail::tiff_reader_traits<ImageData>::reverse)
            {
                std::size_t num_rows = std::min(height_ - y, static_cast<std::size_t>(rows_per_strip_));
                for (std::size_t ty = ty0; ty < ty1; ++ty)
                {
                    // This is in reverse because the TIFFReadRGBAStrip reads are inverted
                    image.set_row(row++, tx0 - x0, tx1 - x0, &strip[(num_rows - ty - 1) * width_ + tx0]);
                }
            }
            else
            {
                for (std::size_t ty = ty0; ty < ty1; ++ty)
                {
                    image.set_row(row++, tx0 - x0, tx1 - x0, &strip[ty * width_ + tx0]);
                }
            }
        }
    }
}

template<typename T>
TIFF* tiff_reader<T>::open(std::istream& input)
{
    if (!tif_)
    {
        tif_ = tiff_ptr(TIFFClientOpen("tiff_input_stream",
                                       "rcm",
                                       reinterpret_cast<thandle_t>(&input),
                                       detail::tiff_read_proc,
                                       detail::tiff_write_proc,
                                       detail::tiff_seek_proc,
                                       detail::tiff_close_proc,
                                       detail::tiff_size_proc,
                                       detail::tiff_map_proc,
                                       detail::tiff_unmap_proc),
                        tiff_closer());
    }
    return tif_.get();
}

} // namespace mapnik
