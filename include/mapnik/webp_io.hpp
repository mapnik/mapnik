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

// webp
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
extern "C"
{
#include <webp/encode.h>
}
#pragma clang diagnostic pop

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
inline int import_image(T2 const& im_in,
                             WebPPicture & pic,
                             bool alpha)
{
    image<typename T2::pixel> const& data = im_in.data();
    std::size_t width = im_in.width();
    std::size_t height = im_in.height();
    int stride = sizeof(typename T2::pixel_type) * width;
    if (data.width() == width &&
        data.height() == height)
    {
        if (alpha)
        {
            return WebPPictureImportRGBA(&pic, data.bytes(), stride);
        }
        else
        {
    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
            return WebPPictureImportRGBX(&pic, data.bytes(), stride);
    #else
            return WebPPictureImportRGBA(&pic, data.bytes(), stride);
    #endif
        }
    }
    else
    {
        // need to copy: https://github.com/mapnik/mapnik/issues/2024
        image_rgba8 im(width,height);
        for (unsigned y = 0; y < height; ++y)
        {
            typename T2::pixel_type const * row_from = im_in.get_row(y);
            image_rgba8::pixel_type * row_to = im.get_row(y);
            std::copy(row_from, row_from + width, row_to);
        }
        if (alpha)
        {
            return WebPPictureImportRGBA(&pic, im.bytes(), stride);
        }
        else
        {
    #if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
            return WebPPictureImportRGBX(&pic, im.bytes(), stride);
    #else
            return WebPPictureImportRGBA(&pic, im.bytes(), stride);
    #endif
        }
    }
}

template <>
inline int import_image(image_rgba8 const& im,
                             WebPPicture & pic,
                             bool alpha)
{
    int stride = sizeof(image_rgba8::pixel_type) * im.width();
    if (alpha)
    {
        return WebPPictureImportRGBA(&pic, im.bytes(), stride);
    }
    else
    {
#if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
        return WebPPictureImportRGBX(&pic, im.bytes(), stride);
#else
        return WebPPictureImportRGBA(&pic, im.bytes(), stride);
#endif
    }
}

template <typename T1, typename T2>
void save_as_webp(T1& file,
                  T2 const& image,
                  WebPConfig const& config,
                  bool alpha)
{
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
    pic.use_argb = !!config.lossless;
    // lossless fast track
    if (pic.use_argb)
    {
        pic.colorspace = static_cast<WebPEncCSP>(pic.colorspace | WEBP_CSP_ALPHA_BIT);
        if (WebPPictureAlloc(&pic)) {
            ok = 1;
            const int width = pic.width;
            const int height = pic.height;
            for (int y = 0; y < height; ++y) {
                typename T2::pixel_type const * row = image.get_row(y);
                for (int x = 0; x < width; ++x) {
                    const unsigned rgba = row[x];
                    unsigned a = (rgba >> 24) & 0xff;
                    unsigned r = rgba & 0xff;
                    unsigned g = (rgba >> 8 ) & 0xff;
                    unsigned b = (rgba >> 16) & 0xff;
                    const uint32_t argb = (a << 24) | (r << 16) | (g << 8) | (b);
                    pic.argb[x + y * pic.argb_stride] = argb;
                }
            }
        }
    }
    else
    {
        // different approach for lossy since ImportYUVAFromRGBA is needed
        // to prepare WebPPicture and working with view pixels is not viable
        ok = import_image(image,pic,alpha);
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
