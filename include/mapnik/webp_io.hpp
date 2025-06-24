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

#ifndef MAPNIK_WEBP_IO_HPP
#define MAPNIK_WEBP_IO_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/image.hpp>
#include <mapnik/util/conversions.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
extern "C" {
#include <webp/encode.h>
}
MAPNIK_DISABLE_WARNING_POP

// stl
#include <algorithm>
#include <stdexcept>
#include <string>

namespace mapnik {

template<typename T>
int webp_stream_write(const uint8_t* data, size_t data_size, const WebPPicture* picture)
{
    T* out = static_cast<T*>(picture->custom_ptr);
    out->write(reinterpret_cast<const char*>(data), data_size);
    return true;
}

std::string MAPNIK_DECL webp_encoding_error(WebPEncodingError error);

template<typename T2>
inline int import_image(T2 const& im_in, WebPPicture& pic, bool alpha)
{
    image<typename T2::pixel> const& data = im_in.data();
    std::size_t width = im_in.width();
    std::size_t height = im_in.height();
    std::size_t stride = sizeof(typename T2::pixel_type) * width;
    if (data.width() == width && data.height() == height)
    {
        if (alpha)
        {
            return WebPPictureImportRGBA(&pic, data.bytes(), static_cast<int>(stride));
        }
        else
        {
#if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
            return WebPPictureImportRGBX(&pic, data.bytes(), static_cast<int>(stride));
#else
            return WebPPictureImportRGBA(&pic, data.bytes(), static_cast<int>(stride));
#endif
        }
    }
    else
    {
        // need to copy: https://github.com/mapnik/mapnik/issues/2024
        image_rgba8 im(width, height);
        for (unsigned y = 0; y < height; ++y)
        {
            typename T2::pixel_type const* row_from = im_in.get_row(y);
            image_rgba8::pixel_type* row_to = im.get_row(y);
            std::copy(row_from, row_from + width, row_to);
        }
        if (alpha)
        {
            return WebPPictureImportRGBA(&pic, im.bytes(), static_cast<int>(stride));
        }
        else
        {
#if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
            return WebPPictureImportRGBX(&pic, im.bytes(), static_cast<int>(stride));
#else
            return WebPPictureImportRGBA(&pic, im.bytes(), static_cast<int>(stride));
#endif
        }
    }
}

template<>
inline int import_image(image_rgba8 const& im, WebPPicture& pic, bool alpha)
{
    std::size_t stride = sizeof(image_rgba8::pixel_type) * im.width();
    if (alpha)
    {
        return WebPPictureImportRGBA(&pic, im.bytes(), static_cast<int>(stride));
    }
    else
    {
#if (WEBP_ENCODER_ABI_VERSION >> 8) >= 1
        return WebPPictureImportRGBX(&pic, im.bytes(), static_cast<int>(stride));
#else
        return WebPPictureImportRGBA(&pic, im.bytes(), static_cast<int>(stride));
#endif
    }
}

template<typename T1, typename T2>
void save_as_webp(T1& file, T2 const& image, WebPConfig const& config, bool alpha)
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
        if (WebPPictureAlloc(&pic))
        {
            ok = 1;
            const int width = pic.width;
            const int height = pic.height;
            for (int y = 0; y < height; ++y)
            {
                typename T2::pixel_type const* row = image.get_row(y);
                for (int x = 0; x < width; ++x)
                {
                    const unsigned rgba = row[x];
                    unsigned a = (rgba >> 24) & 0xff;
                    unsigned r = rgba & 0xff;
                    unsigned g = (rgba >> 8) & 0xff;
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
        ok = import_image(image, pic, alpha);
    }
#else
    ok = import_image(image, pic, alpha);
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
} // namespace mapnik

#endif // MAPNIK_WEBP_IO_HPP
