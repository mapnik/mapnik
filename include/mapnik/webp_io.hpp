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

#ifndef MAPNIK_WEBP_IO_HPP
#define MAPNIK_WEBP_IO_HPP

// mapnik
#include <mapnik/util/conversions.hpp>

// webp
#include <webp/encode.h>

// stl
#include <stdexcept>
#include <string>

// boost
#include <boost/scoped_array.hpp>


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

template <typename T1, typename T2>
void save_as_webp(T1& file,
                  float quality,
                  int method,
                  int lossless,
                  int image_hint,
                  bool alpha,
                  T2 const& image)
{
    WebPConfig config;
    if (!WebPConfigPreset(&config, WEBP_PRESET_DEFAULT, quality))
    {
        throw std::runtime_error("version mismatch");
    }

    // Add additional tuning
    if (method >= 0) config.method = method;
#if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
    config.lossless = !!lossless;
    config.image_hint = static_cast<WebPImageHint>(image_hint);
#else
    #ifdef _MSC_VER
    #pragma NOTE(compiling against webp that does not support lossless flag)
    #else
    #warning "compiling against webp that does not support lossless flag"
    #endif
#endif

    bool valid = WebPValidateConfig(&config);
    if (!valid)
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
#if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
    pic.use_argb = !!lossless;
#endif
    int ok = 0;
    if (alpha)
    {
        int stride = sizeof(typename T2::pixel_type) * image.width();
        ok = WebPPictureImportRGBA(&pic, image.getBytes(), stride);
    }
    else
    {
        int stride = sizeof(typename T2::pixel_type) * image.width();
#if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
        ok = WebPPictureImportRGBX(&pic, image.getBytes(), stride);
#else
        ok = WebPPictureImportRGBA(&pic, image.getBytes(), stride);
#endif
    }

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
