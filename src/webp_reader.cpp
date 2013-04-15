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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/image_reader.hpp>

extern "C"
{
#include <webp/types.h>
#include <webp/decode.h>
}

// boost
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

namespace mapnik
{

class webp_reader : public image_reader
{
private:

    struct config_guard
    {
        config_guard(WebPDecoderConfig & config)
            : config_(config) {}

        ~config_guard()
        {
            WebPFreeDecBuffer(&config_.output);
        }
        WebPDecoderConfig & config_;
    };

    uint8_t const* data_;
    size_t size_;
    unsigned width_;
    unsigned height_;
public:
    explicit webp_reader(char const* data, std::size_t size);
    ~webp_reader();
    unsigned width() const;
    unsigned height() const;
    bool premultiplied_alpha() const { return false; }
    void read(unsigned x,unsigned y,image_data_32& image);
private:
    void init();
};

namespace
{
image_reader* create_webp_reader(char const * data, std::size_t size)
{
    return new webp_reader(data, size);
}

const bool registered = register_image_reader("webp", create_webp_reader);

}

// ctor
webp_reader::webp_reader(char const* data, std::size_t size)
    : data_(reinterpret_cast<uint8_t const*>(data)),
      size_(size),
      width_(0),
      height_(0)
{
    init();
}

// dtor
webp_reader::~webp_reader()
{
    //
}

void webp_reader::init()
{
    int width, height;
    if (!WebPGetInfo(data_, size_, &width, &height))
    {
        throw image_reader_exception("WEBP reader: WebPGetInfo failed");
    }
    width_ = width;
    height_ = height;
}

unsigned webp_reader::width() const
{
    return width_;
}

unsigned webp_reader::height() const
{
    return height_;
}

void webp_reader::read(unsigned x0, unsigned y0,image_data_32& image)
{
    WebPDecoderConfig config;
    config_guard guard(config);
    if (!WebPInitDecoderConfig(&config))
    {
        throw image_reader_exception("WEBP reader: WebPInitDecoderConfig failed");
    }

    config.options.use_cropping = 1;
    config.options.crop_left = x0;
    config.options.crop_top = y0;
    config.options.crop_width = std::min(width_ - x0, image.width());
    config.options.crop_height = std::min(height_ - y0, image.height());

    if (WebPGetFeatures(data_, size_, &config.input) != VP8_STATUS_OK)
    {
        throw image_reader_exception("WEBP reader: WebPGetFeatures failed");
    }

    config.output.colorspace = MODE_RGBA;
    config.output.u.RGBA.rgba = (uint8_t *)image.getBytes();
    config.output.u.RGBA.stride = 4 * image.width();
    config.output.u.RGBA.size = image.width() * image.height() * 4;
    config.output.is_external_memory = 1;
    if (WebPDecode(data_, size_, &config) != VP8_STATUS_OK)
    {
        throw image_reader_exception("WEBP reader: WebPDecode failed");
    }
}

}
