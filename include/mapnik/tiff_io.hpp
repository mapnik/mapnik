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

#ifndef MAPNIK_TIFF_IO_HPP
#define MAPNIK_TIFF_IO_HPP

#include <mapnik/global.hpp>

#include <iostream>

extern "C"
{
#include <tiffio.h>
}

namespace mapnik {

static tsize_t tiff_write_proc(thandle_t fd, tdata_t buf, tsize_t size)
{
    std::ostream* out = (std::ostream*)fd;

    out->write((const char*)buf, size);

    return size;
}

static toff_t tiff_seek_proc(thandle_t fd, toff_t off, int whence)
{
    if (off == 0xFFFFFFFF)
    {
        return 0xFFFFFFFF;
    }

    std::ostream* out = (std::ostream*)fd;

    switch(whence)
    {
    case SEEK_CUR:
        out->seekp(off, std::ios_base::cur);
        break;
    case SEEK_END:
        out->seekp(off, std::ios_base::end);
        break;
    case SEEK_SET:
    default:
        out->seekp(off, std::ios_base::beg);
        break;
    }

    return (toff_t)out->tellp();
}

static int tiff_close_proc(thandle_t fd)
{
    std::ostream* out = (std::ostream*)fd;
    out->flush();
    return 0;
}

static toff_t tiff_size_proc(thandle_t fd)
{
    std::ostream* out = (std::ostream*)fd;
    return (toff_t)out->tellp();
}

static tsize_t tiff_dummy_read_proc(thandle_t fd, tdata_t buf, tsize_t size)
{
    return 0;
}

static void tiff_dummy_unmap_proc(thandle_t fd, tdata_t base, toff_t size)
{
}

static int tiff_dummy_map_proc(thandle_t fd, tdata_t* pbase, toff_t* psize)
{
    return 0;
}

template <typename T1, typename T2>
void save_as_tiff(T1 & file, T2 const& image)
{
    const int width = image.width();
    const int height = image.height();
    const int scanline_size = sizeof(unsigned char) * width * 3;

    TIFF* output = TIFFClientOpen("mapnik_tiff_stream",
                                  "w",
                                  (thandle_t)&file,
                                  tiff_dummy_read_proc,
                                  tiff_write_proc,
                                  tiff_seek_proc,
                                  tiff_close_proc,
                                  tiff_size_proc,
                                  tiff_dummy_map_proc,
                                  tiff_dummy_unmap_proc);

    TIFFSetField(output, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(output, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(output, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
    TIFFSetField(output, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, 1);

    int next_scanline = 0;
    unsigned char* row = reinterpret_cast<unsigned char*>(::operator new(scanline_size));

    while (next_scanline < height)
    {
        const unsigned* imageRow = image.getRow(next_scanline);

        for (int i = 0, index = 0; i < width; ++i)
        {
 #ifdef MAPNIK_BIG_ENDIAN
            row[index++] = (imageRow[i] >> 24) & 0xff;
            row[index++] = (imageRow[i] >> 16) & 0xff;
            row[index++] = (imageRow[i] >> 8) & 0xff;
 #else
            row[index++] = (imageRow[i]) & 0xff;
            row[index++] = (imageRow[i] >> 8) & 0xff;
            row[index++] = (imageRow[i] >> 16) & 0xff;
 #endif
        }

        TIFFWriteScanline(output, row, next_scanline, 0);
        ++next_scanline;
    }

    ::operator delete(row);

    TIFFClose(output);
}

}


#endif // MAPNIK_TIFF_IO_HPP
