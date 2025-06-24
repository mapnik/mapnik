/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_TIFF_IO_HPP
#define MAPNIK_TIFF_IO_HPP

#include <mapnik/global.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/util/variant.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>

extern "C" {
#include <tiffio.h>
#define RealTIFFOpen  TIFFClientOpen
#define RealTIFFClose TIFFClose
}

MAPNIK_DISABLE_WARNING_POP

// std
#include <memory>

#define TIFF_WRITE_SCANLINE 0
#define TIFF_WRITE_STRIPPED 1
#define TIFF_WRITE_TILED    2

namespace mapnik {

static inline tsize_t tiff_write_proc(thandle_t fd, tdata_t buf, tsize_t size)
{
    std::ostream* out = reinterpret_cast<std::ostream*>(fd);
    std::ios::pos_type pos = out->tellp();
    std::streamsize request_size = size;
    if (static_cast<tsize_t>(request_size) != size)
        return static_cast<tsize_t>(-1);
    out->write(reinterpret_cast<const char*>(buf), size);

    if (static_cast<std::streamsize>(pos) == -1)
    {
        return size;
    }
    else
    {
        return static_cast<tsize_t>(out->tellp() - pos);
    }
}

static inline toff_t tiff_seek_proc(thandle_t fd, toff_t off, int whence)
{
    std::ostream* out = reinterpret_cast<std::ostream*>(fd);

    if (out->fail())
        return static_cast<toff_t>(-1);

    if (static_cast<std::streamsize>(out->tellp()) == -1)
        return static_cast<toff_t>(0);

    switch (whence)
    {
        case SEEK_SET:
            out->seekp(off, std::ios_base::beg);
            break;
        case SEEK_CUR:
            out->seekp(off, std::ios_base::cur);
            break;
        case SEEK_END:
            out->seekp(off, std::ios_base::end);
            break;
    }
    // grow std::stringstream buffer (re: libtiff/tif_stream.cxx)
    std::ios::pos_type pos = out->tellp();
    // second check needed for clang (libcxx doesn't set failbit when seeking beyond the current buffer size

    if (out->fail() || static_cast<std::streamoff>(off) != pos)
    {
        std::ios::iostate old_state;
        std::ios::pos_type origin;
        old_state = out->rdstate();
        // reset the fail bit or else tellp() won't work below
        out->clear(out->rdstate() & ~std::ios::failbit);
        switch (whence)
        {
            case SEEK_SET:
            default:
                origin = 0L;
                break;
            case SEEK_CUR:
                origin = out->tellp();
                break;
            case SEEK_END:
                out->seekp(0, std::ios::end);
                origin = out->tellp();
                break;
        }
        // restore original stream state
        out->clear(old_state);

        // only do something if desired seek position is valid
        if ((static_cast<uint64_t>(origin) + off) > 0L)
        {
            uint64_t num_fill;
            // clear the fail bit
            out->clear(out->rdstate() & ~std::ios::failbit);
            // extend the stream to the expected size
            out->seekp(0, std::ios::end);
            num_fill = (static_cast<uint64_t>(origin)) + off - out->tellp();
            for (uint64_t i = 0; i < num_fill; ++i)
                out->put('\0');

            // retry the seek
            out->seekp(static_cast<std::ios::off_type>(static_cast<uint64_t>(origin) + off), std::ios::beg);
        }
    }
    return static_cast<toff_t>(out->tellp());
}

static inline int tiff_close_proc(thandle_t fd)
{
    std::ostream* out = (std::ostream*)fd;
    out->flush();
    return 0;
}

static inline toff_t tiff_size_proc(thandle_t fd)
{
    std::ostream* out = reinterpret_cast<std::ostream*>(fd);
    std::ios::pos_type pos = out->tellp();
    out->seekp(0, std::ios::end);
    std::ios::pos_type len = out->tellp();
    out->seekp(pos);
    return static_cast<toff_t>(len);
}

static inline tsize_t tiff_dummy_read_proc(thandle_t, tdata_t, tsize_t)
{
    return 0;
}

static inline void tiff_dummy_unmap_proc(thandle_t, tdata_t, toff_t) {}

static inline int tiff_dummy_map_proc(thandle_t, tdata_t*, toff_t*)
{
    return 0;
}

struct tiff_config
{
    tiff_config()
        : compression(COMPRESSION_ADOBE_DEFLATE)
        , zlevel(4)
        , tile_width(0)
        , tile_height(0)
        , rows_per_strip(0)
        , method(TIFF_WRITE_STRIPPED)
    {}

    int compression;
    int zlevel;
    int tile_width;  // Tile width of zero means tile the width of the image
    int tile_height; // Tile height of zero means tile the height of the image
    int rows_per_strip;
    int method; // The method to use to write the TIFF.
};

struct tag_setter
{
    tag_setter(TIFF* output, tiff_config const& config)
        : output_(output)
        , config_(config)
    {}

    template<typename T>
    void operator()(T const&) const
    {
        // Assume this would be null type
        throw image_writer_exception("Could not write TIFF - unknown image type provided");
    }

    inline void operator()(image_rgba8 const& data) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 4);
        if (data.get_premultiplied())
        {
            std::uint16_t extras[] = {EXTRASAMPLE_ASSOCALPHA};
            TIFFSetField(output_, TIFFTAG_EXTRASAMPLES, 1, extras);
        }
        else
        {
            std::uint16_t extras[] = {EXTRASAMPLE_UNASSALPHA};
            TIFFSetField(output_, TIFFTAG_EXTRASAMPLES, 1, extras);
        }
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
        }
    }
    inline void operator()(image_gray64 const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 64);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
        }
    }
    inline void operator()(image_gray64s const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 64);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
        }
    }
    inline void operator()(image_gray64f const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 64);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_FLOATINGPOINT);
        }
    }
    inline void operator()(image_gray32 const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 32);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
        }
    }
    inline void operator()(image_gray32s const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 32);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
        }
    }
    inline void operator()(image_gray32f const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 32);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_FLOATINGPOINT);
        }
    }
    inline void operator()(image_gray16 const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 16);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
        }
    }
    inline void operator()(image_gray16s const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 16);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
        }
    }
    inline void operator()(image_gray8 const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
        }
    }
    inline void operator()(image_gray8s const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE || config_.compression == COMPRESSION_ADOBE_DEFLATE ||
            config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);
        }
    }
    inline void operator()(image_null const&) const
    {
        // Assume this would be null type
        throw image_writer_exception("Could not write TIFF - Null image provided");
    }

  private:
    TIFF* output_;
    tiff_config const& config_;
};

inline void set_tiff_config(TIFF* output, tiff_config const& config)
{
    // Set some constant tiff information that doesn't vary based on type of data
    // or image size
    TIFFSetField(output, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    // Set the compression for the TIFF
    TIFFSetField(output, TIFFTAG_COMPRESSION, config.compression);

    if (COMPRESSION_ADOBE_DEFLATE == config.compression || COMPRESSION_DEFLATE == config.compression ||
        COMPRESSION_LZW == config.compression)
    {
        // Set the zip level for the compression
        // http://en.wikipedia.org/wiki/DEFLATE#Encoder.2Fcompressor
        // Changes the time spent trying to compress
        TIFFSetField(output, TIFFTAG_ZIPQUALITY, config.zlevel);
    }
}

template<typename T1, typename T2>
void save_as_tiff(T1& file, T2 const& image, tiff_config const& config)
{
    using pixel_type = typename T2::pixel_type;

    const int width = image.width();
    const int height = image.height();

    TIFF* output = RealTIFFOpen("mapnik_tiff_stream",
                                "wm",
                                (thandle_t)&file,
                                tiff_dummy_read_proc,
                                tiff_write_proc,
                                tiff_seek_proc,
                                tiff_close_proc,
                                tiff_size_proc,
                                tiff_dummy_map_proc,
                                tiff_dummy_unmap_proc);
    if (!output)
    {
        throw image_writer_exception("Could not write TIFF");
    }

    TIFFSetField(output, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(output, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(output, TIFFTAG_IMAGEDEPTH, 1);
    set_tiff_config(output, config);

    // Set tags that vary based on the type of data being provided.
    tag_setter set(output, config);
    set(image);

    // Use specific types of writing methods.
    if (TIFF_WRITE_SCANLINE == config.method)
    {
        // Process Scanline
        TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, 1);

        int next_scanline = 0;
        std::unique_ptr<pixel_type[]> row(new pixel_type[width]);
        while (next_scanline < height)
        {
            std::copy(image.get_row(next_scanline), image.get_row(next_scanline) + width, row.get());
            TIFFWriteScanline(output, row.get(), next_scanline, 0);
            ++next_scanline;
        }
    }
    else if (TIFF_WRITE_STRIPPED == config.method)
    {
        std::size_t rows_per_strip = config.rows_per_strip;
        if (0 == rows_per_strip)
        {
            rows_per_strip = height;
        }
        TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, rows_per_strip);
        std::size_t strip_size = width * rows_per_strip;
        std::unique_ptr<pixel_type[]> strip_buffer(new pixel_type[strip_size]);
        for (int y = 0; y < height; y += rows_per_strip)
        {
            int ty1 = std::min(height, static_cast<int>(y + rows_per_strip)) - y;
            int row = y;
            for (int ty = 0; ty < ty1; ++ty, ++row)
            {
                std::copy(image.get_row(row), image.get_row(row) + width, strip_buffer.get() + ty * width);
            }
            if (TIFFWriteEncodedStrip(output,
                                      TIFFComputeStrip(output, y, 0),
                                      strip_buffer.get(),
                                      strip_size * sizeof(pixel_type)) == -1)
            {
                throw image_writer_exception("Could not write TIFF - TIFF Tile Write failed");
            }
        }
    }
    else if (TIFF_WRITE_TILED == config.method)
    {
        int tile_width = config.tile_width;
        int tile_height = config.tile_height;

        if (0 == tile_height)
        {
            tile_height = height;
            if (height % 16 > 0)
            {
                tile_height = height + 16 - (height % 16);
            }
        }
        if (0 == tile_width)
        {
            tile_width = width;
            if (width % 16 > 0)
            {
                tile_width = width + 16 - (width % 16);
            }
        }
        TIFFSetField(output, TIFFTAG_TILEWIDTH, tile_width);
        TIFFSetField(output, TIFFTAG_TILELENGTH, tile_height);
        TIFFSetField(output, TIFFTAG_TILEDEPTH, 1);
        std::size_t tile_size = tile_width * tile_height;
        std::unique_ptr<pixel_type[]> image_out(new pixel_type[tile_size]);
        int end_y = (height / tile_height + 1) * tile_height;
        int end_x = (width / tile_width + 1) * tile_width;
        end_y = std::min(end_y, height);
        end_x = std::min(end_x, width);

        for (int y = 0; y < end_y; y += tile_height)
        {
            int ty1 = std::min(height, y + tile_height) - y;

            for (int x = 0; x < end_x; x += tile_width)
            {
                // Prefill the entire array with zeros.
                std::fill(image_out.get(), image_out.get() + tile_size, 0);
                int tx1 = std::min(width, x + tile_width);
                int row = y;
                for (int ty = 0; ty < ty1; ++ty, ++row)
                {
                    std::copy(image.get_row(row, x), image.get_row(row, tx1), image_out.get() + ty * tile_width);
                }
                if (TIFFWriteEncodedTile(output,
                                         TIFFComputeTile(output, x, y, 0, 0),
                                         image_out.get(),
                                         tile_size * sizeof(pixel_type)) == -1)
                {
                    throw image_writer_exception("Could not write TIFF - TIFF Tile Write failed");
                }
            }
        }
    }
    // TODO - handle palette images
    // std::vector<mapnik::rgb> const& palette

    //  unsigned short r[256], g[256], b[256];
    //  for (int i = 0; i < (1 << 24); ++i)
    //  {
    //  r[i] = (unsigned short)palette[i * 3 + 0] << 8;
    //  g[i] = (unsigned short)palette[i * 3 + 1] << 8;
    //  b[i] = (unsigned short)palette[i * 3 + 2] << 8;
    //  }
    //  TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
    //  TIFFSetField(output, TIFFTAG_COLORMAP, r, g, b);

    RealTIFFClose(output);
}

} // namespace mapnik

#endif // MAPNIK_TIFF_IO_HPP
