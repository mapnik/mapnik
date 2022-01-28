/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_JPEG_IO_HPP
#define MAPNIK_JPEG_IO_HPP

#if defined(HAVE_JPEG)

#include <new>
#include <ostream>
#include <cstdio>

extern "C" {
#include <jpeglib.h>
}

#define BUFFER_SIZE 4096

namespace jpeg_detail {

typedef struct
{
    struct jpeg_destination_mgr pub;
    std::ostream* out;
    JOCTET* buffer;
} dest_mgr;

inline void init_destination(j_compress_ptr cinfo)
{
    dest_mgr* dest = reinterpret_cast<dest_mgr*>(cinfo->dest);
    dest->buffer = (JOCTET*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_IMAGE, BUFFER_SIZE * sizeof(JOCTET));
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = BUFFER_SIZE;
}

inline boolean empty_output_buffer(j_compress_ptr cinfo)
{
    dest_mgr* dest = reinterpret_cast<dest_mgr*>(cinfo->dest);
    dest->out->write((char*)dest->buffer, BUFFER_SIZE);
    if (!*(dest->out))
        return boolean(0);
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = BUFFER_SIZE;
    return boolean(1);
}

inline void term_destination(j_compress_ptr cinfo)
{
    dest_mgr* dest = reinterpret_cast<dest_mgr*>(cinfo->dest);
    size_t size = BUFFER_SIZE - dest->pub.free_in_buffer;
    if (size > 0)
    {
        dest->out->write((char*)dest->buffer, size);
    }
    dest->out->flush();
}

} // namespace jpeg_detail

namespace mapnik {

template<typename T1, typename T2>
void save_as_jpeg(T1& file, int quality, T2 const& image)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    int width = static_cast<int>(image.width());
    int height = static_cast<int>(image.height());

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    cinfo.dest = (struct jpeg_destination_mgr*)(*cinfo.mem->alloc_small)((j_common_ptr)&cinfo,
                                                                         JPOOL_PERMANENT,
                                                                         sizeof(jpeg_detail::dest_mgr));
    jpeg_detail::dest_mgr* dest = reinterpret_cast<jpeg_detail::dest_mgr*>(cinfo.dest);
    dest->pub.init_destination = jpeg_detail::init_destination;
    dest->pub.empty_output_buffer = jpeg_detail::empty_output_buffer;
    dest->pub.term_destination = jpeg_detail::term_destination;
    dest->out = &file;

    // jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, boolean(1));
    jpeg_start_compress(&cinfo, boolean(1));
    JSAMPROW row_pointer[1];
    JSAMPLE* row = reinterpret_cast<JSAMPLE*>(::operator new(sizeof(JSAMPLE) * width * 3));
    while (cinfo.next_scanline < cinfo.image_height)
    {
        const unsigned* imageRow = image.get_row(cinfo.next_scanline);
        int index = 0;
        for (int i = 0; i < width; ++i)
        {
            row[index++] = (imageRow[i]) & 0xff;
            row[index++] = (imageRow[i] >> 8) & 0xff;
            row[index++] = (imageRow[i] >> 16) & 0xff;
        }
        row_pointer[0] = &row[0];
        (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    ::operator delete(row);

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
}
} // namespace mapnik

#endif

#endif // MAPNIK_JPEG_IO_HPP
