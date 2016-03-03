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

#ifndef MAPNIK_WEBP_IO_HPP
#define MAPNIK_WEBP_IO_HPP

// mapnik
#include <mapnik/image.hpp>
#include <mapnik/util/conversions.hpp>

// agg
#include <agg_pixfmt_rgba.h>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
extern "C"
{
#include <webp/encode.h>
}
#pragma GCC diagnostic pop

// stl
#include <algorithm>
#include <stdexcept>
#include <string>

namespace mapnik {

template <typename T>
int webp_stream_write(const uint8_t* data, size_t data_size, const WebPPicture* picture)
{
    T* out = static_cast<T*>(picture->custom_ptr);
    out->write(reinterpret_cast<const char*>(data), data_size);
    return true;
}

std::string webp_encoding_error(WebPEncodingError error)
{
    std::string os;
    switch (error)
    {
        case VP8_ENC_ERROR_OUT_OF_MEMORY: os = "memory error allocating objects"; break;
        case VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY: os = "memory error while flushing bits"; break;
        case VP8_ENC_ERROR_NULL_PARAMETER: os = "a pointer parameter is NULL"; break;
        case VP8_ENC_ERROR_INVALID_CONFIGURATION: os = "configuration is invalid"; break;
        case VP8_ENC_ERROR_BAD_DIMENSION: os = "picture has invalid width/height"; break;
        case VP8_ENC_ERROR_PARTITION0_OVERFLOW: os = "partition is bigger than 512k"; break;
        case VP8_ENC_ERROR_PARTITION_OVERFLOW: os = "partition is bigger than 16M"; break;
        case VP8_ENC_ERROR_BAD_WRITE: os = "error while flushing bytes"; break;
        case VP8_ENC_ERROR_FILE_TOO_BIG: os = "file is bigger than 4G"; break;
        default:
            mapnik::util::to_string(os,error);
            os = "unknown error (" + os + ")"; break;
    }
    return os;
}

template <typename T2>
static int import_image(T2 const& image,
                        WebPPicture & pic,
                        bool alpha)
{
    uint8_t const* src = reinterpret_cast<uint8_t const*>(image.get_row(0));
    int stride = static_cast<int>(image.row_stride());
    if (alpha)
    {
        return WebPPictureImportRGBA(&pic, src, stride);
    }
    else
    {
#if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
        return WebPPictureImportRGBX(&pic, src, stride);
#else
        return WebPPictureImportRGBA(&pic, src, stride);
#endif
    }
}

template <typename T1, typename T2>
void save_as_webp(T1& file,
                  T2 const& image,
                  WebPConfig const& config,
                  bool alpha)
{
    static_assert(sizeof(typename T2::pixel_type) == 4, "T2 must be rgba8 image/view");

    if (WebPValidateConfig(&config) != 1)
    {
        throw std::runtime_error("Invalid configuration");
    }

    WebPPicture pic;
    if (!WebPPictureInit(&pic))
    {
        throw std::runtime_error("version mismatch");
    }
    pic.width = image.width();
    pic.height = image.height();
    int ok = 0;
#if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
    // always use ARGB input, so that WebPEncode applies proper config.preprocessing
    // https://chromium.googlesource.com/webm/libwebp/+/a6f23c4/src/enc/webpenc.c#337
    pic.use_argb = 1;
    if (!image.get_premultiplied())
    {
        ok = import_image(image, pic, alpha);
    }
    else if ((ok = WebPPictureAlloc(&pic)) != 0)
    {
        for (int y = 0; y < pic.height; ++y)
        {
            uint32_t const* src_row = image.get_row(y);
            uint8_t const* src = reinterpret_cast<uint8_t const*>(src_row);
            uint8_t pix[4];
            auto dst = pic.argb + y * pic.argb_stride;
            for (int x = 0; x < pic.width; ++x, src += 4)
            {
                using m = agg::multiplier2_rgba<agg::rgba8, agg::order_rgba>;
                m::demultiply(pix, src);
                uint32_t r = pix[0];
                uint32_t g = pix[1];
                uint32_t b = pix[2];
                uint32_t a = alpha ? pix[3] : 255;
                dst[x] = (a << 24) | (r << 16) | (g << 8) | (b);
            }
        }
    }
#else
    ok = import_image(image,pic,alpha);
#endif
    if (!ok)
    {
        throw std::runtime_error(webp_encoding_error(pic.error_code));
    }

    pic.writer = webp_stream_write<T1>;
    pic.custom_ptr = &file;
    ok = WebPEncode(&config, &pic);
    WebPPictureFree(&pic);
    if (!ok)
    {
        throw std::runtime_error(webp_encoding_error(pic.error_code));
    }
    file.flush();
}
}

#endif // MAPNIK_WEBP_IO_HPP
