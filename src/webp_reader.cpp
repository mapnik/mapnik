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
// stl
#include <fstream>

namespace mapnik
{

struct external_buffer_policy
{
    external_buffer_policy( uint8_t const* data, std::size_t size)
        : data_(data),
          size_(size) {}

    uint8_t const* data() const
    {
        return data_;
    }

    std::size_t size() const
    {
        return size_;
    }

    uint8_t const* data_;
    std::size_t size_;
};

struct internal_buffer_policy
{
    internal_buffer_policy(std::size_t size)
        : data_((size!=0) ? static_cast<uint8_t*>(::operator new(sizeof(uint8_t) * size)) : 0),
          size_(size)
    {}

    uint8_t * data() const
    {
        return data_;
    }

    std::size_t size() const
    {
        return size_;
    }

    ~internal_buffer_policy()
    {
        ::operator delete(data_), data_=0;
    }

    uint8_t * data_;
    std::size_t size_;
};

template <typename T>
class webp_reader : public image_reader
{
    typedef T buffer_policy_type;
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

    std::unique_ptr<buffer_policy_type> buffer_;
    size_t size_;
    unsigned width_;
    unsigned height_;
public:
    explicit webp_reader(char const* data, std::size_t size);
    explicit webp_reader(std::string const& filename);
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
    return new webp_reader<external_buffer_policy>(data, size);
}

image_reader* create_webp_reader2(std::string const& filename)
{
    return new webp_reader<internal_buffer_policy>(filename);
}


const bool registered = register_image_reader("webp", create_webp_reader);
const bool registered2 = register_image_reader("webp", create_webp_reader2);

}

// ctor
template <typename T>
webp_reader<T>::webp_reader(char const* data, std::size_t size)
    : buffer_(new buffer_policy_type(reinterpret_cast<uint8_t const*>(data), size)),
      width_(0),
      height_(0)
{
    init();
}

template <typename T>
webp_reader<T>::webp_reader(std::string const& filename)
    : buffer_(nullptr),
      size_(0),
      width_(0),
      height_(0)
{
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file)
    {
        throw image_reader_exception("WEBP: Can't read file:" + filename);
    }
    std::streampos beg = file.tellg();
    file.seekg (0, std::ios::end);
    std::streampos end = file.tellg();
    std::size_t file_size = end - beg;
    file.seekg (0, std::ios::beg);

    std::unique_ptr<buffer_policy_type> buffer(new buffer_policy_type(file_size));
    file.read(reinterpret_cast<char*>(buffer->data()), buffer->size());
    if (!file)
    {
        throw image_reader_exception("WEBP: Failed to read:" + filename);
    }

    buffer_ = std::move(buffer);
    init();
}


// dtor
template <typename T>
webp_reader<T>::~webp_reader()
{
    //
}

template <typename T>
void webp_reader<T>::init()
{
    int width, height;
    if (!WebPGetInfo(buffer_->data(), buffer_->size(), &width, &height))
    {
        throw image_reader_exception("WEBP reader: WebPGetInfo failed");
    }
    width_ = width;
    height_ = height;
}

template <typename T>
unsigned webp_reader<T>::width() const
{
    return width_;
}

template <typename T>
unsigned webp_reader<T>::height() const
{
    return height_;
}

template <typename T>
void webp_reader<T>::read(unsigned x0, unsigned y0,image_data_32& image)
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

    if (WebPGetFeatures(buffer_->data(), buffer_->size(), &config.input) != VP8_STATUS_OK)
    {
        throw image_reader_exception("WEBP reader: WebPGetFeatures failed");
    }

    config.output.colorspace = MODE_RGBA;
    config.output.u.RGBA.rgba = (uint8_t *)image.getBytes();
    config.output.u.RGBA.stride = 4 * image.width();
    config.output.u.RGBA.size = image.width() * image.height() * 4;
    config.output.is_external_memory = 1;
    if (WebPDecode(buffer_->data(), buffer_->size(), &config) != VP8_STATUS_OK)
    {
        throw image_reader_exception("WEBP reader: WebPDecode failed");
    }
}

}
