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

#ifndef MAPNIK_TIFF_IO_HPP
#define MAPNIK_TIFF_IO_HPP

#include <mapnik/global.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_data_any.hpp>
#include <mapnik/util/variant.hpp>

extern "C"
{
#include <tiffio.h>
#define RealTIFFOpen TIFFClientOpen
#define RealTIFFClose TIFFClose
}

namespace mapnik {

static inline tsize_t tiff_write_proc(thandle_t fd, tdata_t buf, tsize_t size)
{
    std::ostream* out = reinterpret_cast<std::ostream*>(fd);
    std::ios::pos_type pos = out->tellp();
    std::streamsize request_size = size;
    if (static_cast<tsize_t>(request_size) != size)
        return static_cast<tsize_t>(-1);
    out->write(reinterpret_cast<const char*>(buf), size);

    if( static_cast<std::streamsize>(pos) == -1 )
    {
        return size;
    }
    else
    {
        return static_cast<tsize_t>(out->tellp()-pos);
    }
}

static inline toff_t tiff_seek_proc(thandle_t fd, toff_t off, int whence)
{
    std::ostream* out = reinterpret_cast<std::ostream*>(fd);

    if( out->fail() )
        return static_cast<toff_t>(-1);

    if( static_cast<std::streamsize>(out->tellp()) == -1)
        return static_cast< toff_t >( 0 );

    switch(whence)
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

    if( out->fail() || static_cast<std::streamoff>(off) != pos)
    {
        std::ios::iostate old_state;
        std::ios::pos_type  origin;
        old_state = out->rdstate();
        // reset the fail bit or else tellp() won't work below
        out->clear(out->rdstate() & ~std::ios::failbit);
        switch( whence )
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
        if( (static_cast<uint64_t>(origin) + off) > 0L)
        {
            uint64_t num_fill;
            // clear the fail bit
            out->clear(out->rdstate() & ~std::ios::failbit);
            // extend the stream to the expected size
            out->seekp(0, std::ios::end);
            num_fill = (static_cast<uint64_t>(origin)) + off - out->tellp();
            for( uint64_t i = 0; i < num_fill; ++i)
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

static inline tsize_t tiff_dummy_read_proc(thandle_t , tdata_t , tsize_t)
{
    return 0;
}

static inline void tiff_dummy_unmap_proc(thandle_t , tdata_t , toff_t) {}

static inline int tiff_dummy_map_proc(thandle_t , tdata_t*, toff_t* )
{
    return 0;
}

struct tiff_config
{
    tiff_config()
        : compression(COMPRESSION_ADOBE_DEFLATE),
        zlevel(4),
        scanline(false) {}

    int compression;
    int zlevel;
    bool scanline;
};

struct tag_setter : public mapnik::util::static_visitor<>
{
    tag_setter(TIFF * output, tiff_config & config)
        : output_(output),
          config_(config) {}

    template <typename T>
    void operator() (T const&) const
    {
        // Assume this would be null type
        throw ImageWriterException("Could not write TIFF - unknown image type provided");
    }

    inline void operator() (image_data_rgba8 const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 4);
        uint16 extras[] = { EXTRASAMPLE_UNASSALPHA };
        TIFFSetField(output_, TIFFTAG_EXTRASAMPLES, 1, extras);
        if (config_.compression == COMPRESSION_DEFLATE
                || config_.compression == COMPRESSION_ADOBE_DEFLATE
                || config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);

        }
    }
    inline void operator() (image_data_gray32f const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 32);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE
                || config_.compression == COMPRESSION_ADOBE_DEFLATE
                || config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_FLOATINGPOINT);
        }
    }
    inline void operator() (image_data_gray16 const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 16);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE
                || config_.compression == COMPRESSION_ADOBE_DEFLATE
                || config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);

        }
    }
    inline void operator() (image_data_gray8 const&) const
    {
        TIFFSetField(output_, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(output_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(output_, TIFFTAG_BITSPERSAMPLE, 8);
        TIFFSetField(output_, TIFFTAG_SAMPLESPERPIXEL, 1);
        if (config_.compression == COMPRESSION_DEFLATE
                || config_.compression == COMPRESSION_ADOBE_DEFLATE
                || config_.compression == COMPRESSION_LZW)
        {
            TIFFSetField(output_, TIFFTAG_PREDICTOR, PREDICTOR_HORIZONTAL);

        }
    }
    inline void operator() (image_data_null const&) const
    {
        // Assume this would be null type
        throw ImageWriterException("Could not write TIFF - Null image provided");
    }

    private:
        TIFF * output_;
        tiff_config config_;
};

void set_tiff_config(TIFF* output, tiff_config & config)
{
    // Set some constant tiff information that doesn't vary based on type of data
    // or image size
    TIFFSetField(output, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    // Set the compression for the TIFF
    TIFFSetField(output, TIFFTAG_COMPRESSION, config.compression);

    if (COMPRESSION_ADOBE_DEFLATE == config.compression || COMPRESSION_DEFLATE == config.compression)
    {
        // Set the zip level for the compression
        // http://en.wikipedia.org/wiki/DEFLATE#Encoder.2Fcompressor
        // Changes the time spent trying to compress
        TIFFSetField(output, TIFFTAG_ZIPQUALITY, config.zlevel);
    }
}

template <typename T1, typename T2>
void save_as_tiff(T1 & file, T2 const& image, tiff_config & config)
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
    if (! output)
    {
        throw ImageWriterException("Could not write TIFF");
    }

    TIFFSetField(output, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(output, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(output, TIFFTAG_IMAGEDEPTH, 1);

    set_tiff_config(output, config);

    // Set tags that vary based on the type of data being provided.
    tag_setter set(output, config);
    set(image);
    //util::apply_visitor(set, image);

    // If the image is greater then 8MB uncompressed, then lets use scanline rather then
    // tile. TIFF also requires that all TIFFTAG_TILEWIDTH and TIFF_TILELENGTH all be
    // a multiple of 16, if they are not we will use scanline.
    if (image.getSize() > 8 * 32 * 1024 * 1024
            || width % 16 != 0
            || height % 16 != 0
            || config.scanline)
    {
        // Process Scanline
        TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, 1);

        int next_scanline = 0;
        std::unique_ptr<pixel_type[]> row (new pixel_type[image.width()]);
        while (next_scanline < height)
        {
            std::copy(image.getRow(next_scanline), image.getRow(next_scanline) + image.width(), row.get());
            //typename T2::pixel_type * row = const_cast<typename T2::pixel_type *>(image.getRow(next_scanline));
            TIFFWriteScanline(output, row.get(), next_scanline, 0);
            ++next_scanline;
        }
    }
    else
    {
        TIFFSetField(output, TIFFTAG_TILEWIDTH, width);
        TIFFSetField(output, TIFFTAG_TILELENGTH, height);
        TIFFSetField(output, TIFFTAG_TILEDEPTH, 1);
        // Process as tiles
        std::size_t tile_size = width * height;
        pixel_type const * image_data_in = image.getData();
        std::unique_ptr<pixel_type[]> image_data_out (new pixel_type[tile_size]);
        std::copy(image_data_in, image_data_in + tile_size, image_data_out.get());
        //typename T2::pixel_type * image_data = const_cast<typename T2::pixel_type *>(image.getData());
        TIFFWriteTile(output, image_data_out.get(), 0, 0, 0, 0);
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

}

#endif // MAPNIK_TIFF_IO_HPP
