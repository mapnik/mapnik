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

// mapnik
#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/color.hpp>
// avif
#include <avif/avif_cxx.h>
// std
#include <cstdio>
#include <memory>
#include <fstream>

namespace mapnik {

template<typename T>
class avif_reader : public image_reader
{
  public:
    using buffer_policy_type = T;

  private:
    avif::DecoderPtr decoder_{avifDecoderCreate()};
    std::unique_ptr<buffer_policy_type> buffer_;
    size_t size_ = 0;
    unsigned width_ = 0;
    unsigned height_ = 0;
    unsigned depth_ = 8;
    bool has_alpha_ = false;

  public:
    explicit avif_reader(std::string const& filename);
    explicit avif_reader(char const* data, size_t size);
    ~avif_reader();
    unsigned width() const final;
    unsigned height() const final;
    std::optional<box2d<double>> bounding_box() const override final;
    inline bool has_alpha() const final { return has_alpha_; }
    void read(unsigned x, unsigned y, image_rgba8& image) final;
    image_any read(unsigned x, unsigned y, unsigned width, unsigned height) final;

  private:
    void init();
};

image_reader* create_avif_reader(std::string const& filename)
{
    return new avif_reader<internal_buffer_policy>(filename);
}

image_reader* create_avif_reader2(char const* data, size_t size)
{
    return new avif_reader<external_buffer_policy>(data, size);
}
void register_avif_reader()
{
    [[maybe_unused]] bool const registered = register_image_reader("avif", create_avif_reader);
    [[maybe_unused]] bool const registered2 = register_image_reader("avif", create_avif_reader2);
}

// ctors
template<typename T>
avif_reader<T>::avif_reader(std::string const& filename)
    : buffer_(nullptr)
{
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file)
    {
        throw image_reader_exception("AVIF: Can't read file:" + filename);
    }
    std::streampos beg = file.tellg();
    file.seekg(0, std::ios::end);
    std::streampos end = file.tellg();
    std::size_t file_size = end - beg;
    file.seekg(0, std::ios::beg);

    auto buffer = std::make_unique<buffer_policy_type>(file_size);
    file.read(reinterpret_cast<char*>(buffer->data()), buffer->size());
    if (!file)
    {
        throw image_reader_exception("AVIF: Failed to read:" + filename);
    }

    buffer_ = std::move(buffer);
    init();
}

template<typename T>
avif_reader<T>::avif_reader(char const* data, size_t size)
    : buffer_(new buffer_policy_type(reinterpret_cast<uint8_t const*>(data), size))
{
    init();
}

// dtor
template<typename T>
avif_reader<T>::~avif_reader()
{}

template<typename T>
void avif_reader<T>::init()
{
    avifResult result = avifDecoderSetIOMemory(decoder_.get(), buffer_->data(), buffer_->size());
    if (result != AVIF_RESULT_OK)
    {
        throw image_reader_exception(std::string("AVIF Reader:") + avifResultToString(result));
    }
    result = avifDecoderParse(decoder_.get());
    if (result != AVIF_RESULT_OK)
    {
        throw image_reader_exception(std::string("AVIF Reader:") + avifResultToString(result));
    }
    width_ = decoder_->image->width;
    height_ = decoder_->image->height;
    depth_ = decoder_->image->depth;
    has_alpha_ = decoder_->alphaPresent;
}

template<typename T>
unsigned avif_reader<T>::width() const
{
    return width_;
}

template<typename T>
unsigned avif_reader<T>::height() const
{
    return height_;
}

template<typename T>
std::optional<box2d<double>> avif_reader<T>::bounding_box() const
{
    return std::nullopt;
}

template<typename T>
void avif_reader<T>::read(unsigned x0, unsigned y0, image_rgba8& image)
{
    bool cropped = x0 != 0 || y0 != 0 || image.width() != width_ || image.height() != height_;
    avifRGBImage rgb;
    std::memset(&rgb, 0, sizeof(rgb));
    // read first image
    if (avifDecoderNextImage(decoder_.get()) == AVIF_RESULT_OK)
    {
        if (!cropped)
        {
            avifRGBImageSetDefaults(&rgb, decoder_->image);
            std::uint32_t const pixelSize = avifRGBImagePixelSize(&rgb);
            std::uint32_t const rowBytes = rgb.width * pixelSize;
            rgb.pixels = image.bytes();
            rgb.rowBytes = rowBytes;
            avifResult result = avifImageYUVToRGB(decoder_->image, &rgb);
            if (result != AVIF_RESULT_OK)
            {
                throw image_reader_exception(std::string("AVIF Reader:") + avifResultToString(result));
            }
        }
        else
        {
            avif::ImagePtr crop{avifImageCreateEmpty()};
            avifCropRect rect = {x0, y0, width_, height_};
            avifResult result = avifImageSetViewRect(crop.get(), decoder_->image, &rect);
            if (result != AVIF_RESULT_OK)
            {
                image_reader_exception(std::string("AVIF Reader:") + avifResultToString(result));
            }
            avifRGBImageSetDefaults(&rgb, crop.get());
            std::uint32_t const pixelSize = avifRGBImagePixelSize(&rgb);
            std::uint32_t const rowBytes = rgb.width * pixelSize;
            rgb.pixels = image.bytes();
            rgb.rowBytes = rowBytes;
            result = avifImageYUVToRGB(crop.get(), &rgb);
            if (result != AVIF_RESULT_OK)
            {
                image_reader_exception(std::string("AVIF Reader:") + avifResultToString(result));
            }
        }
    }
}

template<typename T>
image_any avif_reader<T>::read(unsigned x, unsigned y, unsigned width, unsigned height)
{
    image_rgba8 data(width, height, true, true);
    read(x, y, data);
    return image_any(std::move(data));
}

} // namespace mapnik
