/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
#include <mapnik/util/char_array_buffer.hpp>

extern "C" {
#include <png.h>
}

// stl
#include <cstring>
#include <memory>
#include <fstream>

namespace mapnik {

template<typename T>
class png_reader : public image_reader
{
    using source_type = T;
    using input_stream = std::istream;

    struct png_struct_guard
    {
        png_struct_guard(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr)
            : p_(png_ptr_ptr)
            , i_(info_ptr_ptr)
        {}

        ~png_struct_guard() { png_destroy_read_struct(p_, i_, 0); }
        png_structpp p_;
        png_infopp i_;
    };

  private:

    source_type source_;
    input_stream stream_;
    unsigned width_;
    unsigned height_;
    int bit_depth_;
    int color_type_;
    bool has_alpha_;

  public:
    explicit png_reader(std::string const& filename);
    png_reader(char const* data, std::size_t size);
    ~png_reader();
    unsigned width() const final;
    unsigned height() const final;
    std::optional<box2d<double>> bounding_box() const final;
    inline bool has_alpha() const final { return has_alpha_; }
    void read(unsigned x, unsigned y, image_rgba8& image) final;
    image_any read(unsigned x, unsigned y, unsigned width, unsigned height) final;

  private:
    void init();
    static void png_read_data(png_structp png_ptr, png_bytep data, png_size_t length);
};

image_reader* create_png_reader(std::string const& filename)
{
    return new png_reader<std::filebuf>(filename);
}

image_reader* create_png_reader2(char const* data, std::size_t size)
{
    return new png_reader<mapnik::util::char_array_buffer>(data, size);
}

void register_png_reader()
{
    [[maybe_unused]] const bool registered = register_image_reader("png", create_png_reader);
    [[maybe_unused]] const bool registered2 = register_image_reader("png", create_png_reader2);
}

void user_error_fn(png_structp /*png_ptr*/, png_const_charp error_msg)
{
    throw image_reader_exception(std::string("failed to read invalid png: '") + error_msg + "'");
}

void user_warning_fn(png_structp /*png_ptr*/, png_const_charp warning_msg)
{
    MAPNIK_LOG_DEBUG(png_reader) << "libpng warning: '" << warning_msg << "'";
}

template<typename T>
void png_reader<T>::png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    input_stream* fin = reinterpret_cast<input_stream*>(png_get_io_ptr(png_ptr));
    fin->read(reinterpret_cast<char*>(data), length);
    std::streamsize read_count = fin->gcount();
    if (read_count < 0 || static_cast<png_size_t>(read_count) != length)
    {
        png_error(png_ptr, "Read Error");
    }
}

template<typename T>
png_reader<T>::png_reader(std::string const& filename)
    : source_()
    , stream_(&source_)
    , width_(0)
    , height_(0)
    , bit_depth_(0)
    , color_type_(0)
    , has_alpha_(false)
{
    source_.open(filename, std::ios_base::in | std::ios_base::binary);
    if (!source_.is_open())
        throw image_reader_exception("PNG reader: cannot open file '" + filename + "'");
    init();
}

template<typename T>
png_reader<T>::png_reader(char const* data, std::size_t size)
    : source_(data, size)
    , stream_(&source_)
    , width_(0)
    , height_(0)
    , bit_depth_(0)
    , color_type_(0)
    , has_alpha_(false)
{
    if (!stream_)
        throw image_reader_exception("PNG reader: cannot open image stream");
    init();
}

template<typename T>
png_reader<T>::~png_reader()
{}

template<typename T>
void png_reader<T>::init()
{
    png_byte header[8];
    std::memset(header, 0, 8);
    stream_.read(reinterpret_cast<char*>(header), 8);
    if (stream_.gcount() != 8)
    {
        throw image_reader_exception("PNG reader: Could not read image");
    }
    int is_png = !png_sig_cmp(header, 0, 8);
    if (!is_png)
    {
        throw image_reader_exception("File or stream is not a png");
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (!png_ptr)
    {
        throw image_reader_exception("failed to allocate png_ptr");
    }

    // catch errors in a custom way to avoid the need for setjmp
    png_set_error_fn(png_ptr, png_get_error_ptr(png_ptr), user_error_fn, user_warning_fn);

    png_infop info_ptr;
    png_struct_guard sguard(&png_ptr, &info_ptr);
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        throw image_reader_exception("failed to create info_ptr");

    png_set_read_fn(png_ptr, (png_voidp)&stream_, png_read_data);

    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth_, &color_type_, 0, 0, 0);
    has_alpha_ = (color_type_ & PNG_COLOR_MASK_ALPHA) || png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS);
    width_ = width;
    height_ = height;

    MAPNIK_LOG_DEBUG(png_reader) << "png_reader: bit_depth=" << bit_depth_ << ",color_type=" << color_type_;
}

template<typename T>
unsigned png_reader<T>::width() const
{
    return width_;
}

template<typename T>
unsigned png_reader<T>::height() const
{
    return height_;
}

template<typename T>
std::optional<box2d<double>> png_reader<T>::bounding_box() const
{
    return std::nullopt;
}

template<typename T>
void png_reader<T>::read(unsigned x0, unsigned y0, image_rgba8& image)
{
    stream_.clear();
    stream_.seekg(0, std::ios_base::beg);

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (!png_ptr)
    {
        throw image_reader_exception("failed to allocate png_ptr");
    }

    // catch errors in a custom way to avoid the need for setjmp
    png_set_error_fn(png_ptr, png_get_error_ptr(png_ptr), user_error_fn, user_warning_fn);

    png_infop info_ptr;
    png_struct_guard sguard(&png_ptr, &info_ptr);
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        throw image_reader_exception("failed to create info_ptr");

    png_set_read_fn(png_ptr, (png_voidp)&stream_, png_read_data);
    png_read_info(png_ptr, info_ptr);

    if (color_type_ == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(png_ptr);
    if (color_type_ == PNG_COLOR_TYPE_GRAY && bit_depth_ < 8)
        png_set_expand(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_expand(png_ptr);
    if (bit_depth_ == 16)
        png_set_strip_16(png_ptr);
    if (color_type_ == PNG_COLOR_TYPE_GRAY || color_type_ == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    // quick hack -- only work in >=libpng 1.2.7
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER); // rgba

    double gamma;
    if (png_get_gAMA(png_ptr, info_ptr, &gamma))
        png_set_gamma(png_ptr, 2.2, gamma);

    if (x0 == 0 && y0 == 0 && image.width() >= width_ && image.height() >= height_)
    {
        if (png_get_interlace_type(png_ptr, info_ptr) == PNG_INTERLACE_ADAM7)
        {
            png_set_interlace_handling(png_ptr); // FIXME: libpng bug?
            // according to docs png_read_image
            // "..automatically handles interlacing,
            // so you don't need to call png_set_interlace_handling()"
        }
        png_read_update_info(png_ptr, info_ptr);
        // we can read whole image at once
        // alloc row pointers
        const std::unique_ptr<png_bytep[]> rows(new png_bytep[height_]);
        for (unsigned i = 0; i < height_; ++i)
            rows[i] = (png_bytep)image.get_row(i);
        png_read_image(png_ptr, rows.get());
    }
    else
    {
        png_read_update_info(png_ptr, info_ptr);
        unsigned w = std::min(unsigned(image.width()), width_ - x0);
        unsigned h = std::min(unsigned(image.height()), height_ - y0);
        unsigned rowbytes = png_get_rowbytes(png_ptr, info_ptr);
        const std::unique_ptr<png_byte[]> row(new png_byte[rowbytes]);
        // START read image rows
        for (unsigned i = 0; i < height_; ++i)
        {
            png_read_row(png_ptr, row.get(), 0);
            if (i >= y0 && i < (y0 + h))
            {
                image.set_row(i - y0, reinterpret_cast<unsigned*>(&row[x0 * 4]), w);
            }
        }
        // END
    }
    png_read_end(png_ptr, 0);
}

template<typename T>
image_any png_reader<T>::read(unsigned x, unsigned y, unsigned width, unsigned height)
{
    image_rgba8 data(width, height);
    read(x, y, data);
    return image_any(std::move(data));
}

} // namespace mapnik
