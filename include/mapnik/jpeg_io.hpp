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

#ifndef MAPNIK_JPEG_IO_HPP
#define MAPNIK_JPEG_IO_HPP

#if defined(HAVE_JPEG)

// agg
#include <agg_pixfmt_rgba.h>

// std
#include <new>
#include <ostream>
#include <cstdio>

extern "C"
{
#include <jpeglib.h>
}

#define BUFFER_SIZE 4096

namespace jpeg_detail {

typedef struct
{
    struct jpeg_destination_mgr pub;
    std::ostream * out;
    JOCTET * buffer;
} dest_mgr;

template <typename T, typename J>
T* alloc_small(J* info, int pool_id, size_t count = 1)
{
    auto cinfo = reinterpret_cast<j_common_ptr>(info);
    auto size = count * sizeof(T);
    auto buf = cinfo->mem->alloc_small(cinfo, pool_id, size);
    return reinterpret_cast<T*>(buf);
}

inline void init_destination( j_compress_ptr cinfo)
{
    dest_mgr * dest = reinterpret_cast<dest_mgr*>(cinfo->dest);
    dest->buffer = alloc_small<JOCTET>(cinfo, JPOOL_IMAGE, BUFFER_SIZE);
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = BUFFER_SIZE;
}

inline boolean empty_output_buffer (j_compress_ptr cinfo)
{
    dest_mgr * dest = reinterpret_cast<dest_mgr*>(cinfo->dest);
    dest->out->write((char*)dest->buffer, BUFFER_SIZE);
    if (!*(dest->out)) return boolean(0);
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = BUFFER_SIZE;
    return boolean(1);
}

inline void term_destination( j_compress_ptr cinfo)
{
    dest_mgr * dest = reinterpret_cast<dest_mgr*>(cinfo->dest);
    size_t size  = BUFFER_SIZE - dest->pub.free_in_buffer;
    if (size > 0)
    {
        dest->out->write((char*)dest->buffer, size);
    }
    dest->out->flush();
}

}

namespace mapnik {

template <typename T1, typename T2>
void save_as_jpeg(T1 & file, int quality, T2 const& image, unsigned buf_rows = 1)
{
    static_assert(sizeof(typename T2::pixel_type) == 4, "T2 must be rgba8 image/view");

    using jpeg_detail::alloc_small;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    JDIMENSION width = static_cast<JDIMENSION>(image.width());
    JDIMENSION height = static_cast<JDIMENSION>(image.height());

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    auto dest = alloc_small<jpeg_detail::dest_mgr>(&cinfo, JPOOL_PERMANENT);
    cinfo.dest = &dest->pub;
    dest->pub.init_destination = jpeg_detail::init_destination;
    dest->pub.empty_output_buffer = jpeg_detail::empty_output_buffer;
    dest->pub.term_destination = jpeg_detail::term_destination;
    dest->out = &file;

    //jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, boolean(1));
    jpeg_start_compress(&cinfo, boolean(1));

    JSAMPARRAY scanlines = cinfo.mem->alloc_sarray
        ((j_common_ptr)&cinfo, JPOOL_IMAGE, 3 * width + 1, buf_rows);
        // +1 overflow sample on each row for alpha before demultiply

    for (JDIMENSION y = cinfo.next_scanline; y < cinfo.image_height; )
    {
        uint32_t const* src_row = image.get_row(y);
        uint8_t const* src = reinterpret_cast<uint8_t const*>(src_row);
        JDIMENSION yy = y++ - cinfo.next_scanline;
        JSAMPLE* dst = scanlines[yy++];
        JSAMPLE* dst_end = dst + width * 3;

        if (image.get_premultiplied())
        {
            for (; dst < dst_end; dst += 3, src += 4)
            {
                using m = agg::multiplier2_rgba<agg::rgba8, agg::order_rgba>;
                m::demultiply(dst, src);
            }
        }
        else
        {
            for (; dst < dst_end; dst += 3, src += 4)
            {
                dst[0] = src[0];
                dst[1] = src[1];
                dst[2] = src[2];
            }
        }

        if (yy >= buf_rows || y >= cinfo.image_height)
        {
            jpeg_write_scanlines(&cinfo, scanlines, yy);
            y = cinfo.next_scanline;
        }
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
}
}

#endif

#endif // MAPNIK_JPEG_IO_HPP
