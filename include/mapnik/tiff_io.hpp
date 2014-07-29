/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

extern "C"
{
#ifdef HAVE_GEOTIFF
#include <xtiffio.h>
#include <geotiffio.h>
#define RealTIFFOpen XTIFFClientOpen
#define RealTIFFClose XTIFFClose
#else
#include <tiffio.h>
#define RealTIFFOpen TIFFClientOpen
#define RealTIFFClose TIFFClose
#endif
}

namespace mapnik {

static tsize_t tiff_write_proc(thandle_t fd, tdata_t buf, tsize_t size)
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

static toff_t tiff_seek_proc(thandle_t fd, toff_t off, int whence)
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

static int tiff_close_proc(thandle_t fd)
{
    std::ostream* out = (std::ostream*)fd;
    out->flush();
    return 0;
}

static toff_t tiff_size_proc(thandle_t fd)
{
    std::ostream* out = reinterpret_cast<std::ostream*>(fd);
    std::ios::pos_type pos = out->tellp();
    out->seekp(0, std::ios::end);
    std::ios::pos_type len = out->tellp();
    out->seekp(pos);
    return static_cast<toff_t>(len);
}

static tsize_t tiff_dummy_read_proc(thandle_t , tdata_t , tsize_t)
{
    return 0;
}

static void tiff_dummy_unmap_proc(thandle_t , tdata_t , toff_t)
{
}

static int tiff_dummy_map_proc(thandle_t , tdata_t*, toff_t* )
{
    return 0;
}

template <typename T1, typename T2>
void save_as_tiff(T1 & file, T2 const& image)
{
    const int width = image.width();
    const int height = image.height();
    const int scanline_size = sizeof(unsigned char) * width * 3;


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
    TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
    TIFFSetField(output, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, 1);

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

#ifdef HAVE_GEOTIFF
    GTIF* geotiff = GTIFNew(output);
    if (! geotiff)
    {
        // throw ?
    }

    GTIFKeySet(geotiff, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelGeographic);
    GTIFKeySet(geotiff, GTRasterTypeGeoKey, TYPE_SHORT, 1, RasterPixelIsPoint);
    GTIFKeySet(geotiff, GeographicTypeGeoKey, TYPE_SHORT, 1, 4326); // parameter needed !
    GTIFKeySet(geotiff, GeogAngularUnitsGeoKey, TYPE_SHORT, 1, Angular_Degree);
    GTIFKeySet(geotiff, GeogLinearUnitsGeoKey, TYPE_SHORT, 1, Linear_Meter);

    double lowerLeftLon = 130.0f; // parameter needed !
    double upperRightLat = 32.0; // parameter needed !
    double tiepoints[] = { 0.0, 0.0, 0.0, lowerLeftLon, upperRightLat, 0.0 };
    TIFFSetField(output, TIFFTAG_GEOTIEPOINTS, sizeof(tiepoints)/sizeof(double), tiepoints);

    double pixelScaleX = 0.0001; // parameter needed !
    double pixelScaleY = 0.0001; // parameter needed !
    double pixscale[] = { pixelScaleX, pixelScaleY, 0.0 };
    TIFFSetField(output, TIFFTAG_GEOPIXELSCALE, sizeof(pixscale)/sizeof(double), pixscale);
#endif

    int next_scanline = 0;
    unsigned char* row = reinterpret_cast<unsigned char*>(::operator new(scanline_size));

    while (next_scanline < height)
    {
        const unsigned* imageRow = image.getRow(next_scanline);

        for (int i = 0, index = 0; i < width; ++i)
        {
            row[index++] = (imageRow[i]) & 0xff;
            row[index++] = (imageRow[i] >> 8) & 0xff;
            row[index++] = (imageRow[i] >> 16) & 0xff;
        }

        TIFFWriteScanline(output, row, next_scanline, 0);
        ++next_scanline;
    }

    ::operator delete(row);

#ifdef HAVE_GEOTIFF
    GTIFWriteKeys(geotiff);
    GTIFFree(geotiff);
#endif

    RealTIFFClose(output);
}

}

#endif // MAPNIK_TIFF_IO_HPP
