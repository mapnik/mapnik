/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#include <mapnik/debug.hpp>
#include <mapnik/image_reader.hpp>

extern "C"
{
#include <png.h>
}

#include <boost/scoped_array.hpp>
#include <boost/utility.hpp>

namespace mapnik
{
class png_reader : public image_reader, boost::noncopyable
{
private:
    std::string fileName_;
    unsigned width_;
    unsigned height_;
    int bit_depth_;
    int color_type_;
public:
    explicit png_reader(std::string const& fileName);
    ~png_reader();
    unsigned width() const;
    unsigned height() const;
    bool premultiplied_alpha() const { return false; } //http://www.libpng.org/pub/png/spec/1.1/PNG-Rationale.html
    void read(unsigned x,unsigned y,image_data_32& image);
private:
    void init();
};

namespace
{
image_reader* create_png_reader(std::string const& file)
{
    return new png_reader(file);
}
const bool registered = register_image_reader("png",create_png_reader);
}

png_reader::png_reader(std::string const& fileName)
    : fileName_(fileName),
      width_(0),
      height_(0),
      bit_depth_(0),
      color_type_(0)
{
    init();
}

png_reader::~png_reader() {}

void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
    throw image_reader_exception("failed to read invalid png");
}

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
    MAPNIK_LOG_DEBUG(png_reader) << "libpng warning: '" << warning_msg << "'";
}

static void
png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_size_t check;
    check = (png_size_t)fread(data, (png_size_t)1, length,
                              (FILE *)png_get_io_ptr(png_ptr));

    if (check != length)
    {
        png_error(png_ptr, "Read Error");
    }
}

void png_reader::init()
{
    FILE *fp=fopen(fileName_.c_str(),"rb");
    if (!fp) throw image_reader_exception("cannot open image file "+fileName_);
    png_byte header[8];
    memset(header,0,8);
    if ( fread(header,1,8,fp) != 8)
    {
        fclose(fp);
        throw image_reader_exception("Could not read " + fileName_);
    }
    int is_png=!png_sig_cmp(header,0,8);
    if (!is_png)
    {
        fclose(fp);
        throw image_reader_exception(fileName_ + " is not a png file");
    }
    png_structp png_ptr = png_create_read_struct
        (PNG_LIBPNG_VER_STRING,0,0,0);

    if (!png_ptr)
    {
        fclose(fp);
        throw image_reader_exception("failed to allocate png_ptr");
    }

    // catch errors in a custom way to avoid the need for setjmp
    png_set_error_fn(png_ptr, png_get_error_ptr(png_ptr), user_error_fn, user_warning_fn);

    png_infop info_ptr;
    try
    {
        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
            png_destroy_read_struct(&png_ptr,0,0);
            fclose(fp);
            throw image_reader_exception("failed to create info_ptr");
        }
    }
    catch (std::exception const& ex)
    {
        png_destroy_read_struct(&png_ptr,0,0);
        fclose(fp);
        throw;
    }

    png_set_read_fn(png_ptr, (png_voidp)fp, png_read_data);

    png_set_sig_bytes(png_ptr,8);
    png_read_info(png_ptr, info_ptr);

    png_uint_32  width, height;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth_, &color_type_,0,0,0);

    width_=width;
    height_=height;

    MAPNIK_LOG_DEBUG(png_reader) << "png_reader: bit_depth=" << bit_depth_ << ",color_type=" << color_type_;

    png_destroy_read_struct(&png_ptr,&info_ptr,0);
    fclose(fp);
}

unsigned png_reader::width() const
{
    return width_;
}

unsigned png_reader::height() const
{
    return height_;
}

void png_reader::read(unsigned x0, unsigned y0,image_data_32& image)
{
    FILE *fp=fopen(fileName_.c_str(),"rb");
    if (!fp) throw image_reader_exception("cannot open image file "+fileName_);

    png_structp png_ptr = png_create_read_struct
        (PNG_LIBPNG_VER_STRING,0,0,0);

    if (!png_ptr)
    {
        fclose(fp);
        throw image_reader_exception("failed to allocate png_ptr");
    }

    // catch errors in a custom way to avoid the need for setjmp
    png_set_error_fn(png_ptr, png_get_error_ptr(png_ptr), user_error_fn, user_warning_fn);

    png_infop info_ptr;
    try
    {
        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
            png_destroy_read_struct(&png_ptr,0,0);
            fclose(fp);
            throw image_reader_exception("failed to create info_ptr");
        }
    }
    catch (std::exception const& ex)
    {
        png_destroy_read_struct(&png_ptr,0,0);
        fclose(fp);
        throw;
    }

    png_set_read_fn(png_ptr, (png_voidp)fp, png_read_data);
    png_read_info(png_ptr, info_ptr);

    if (color_type_ == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(png_ptr);
    if (color_type_ == PNG_COLOR_TYPE_GRAY && bit_depth_ < 8)
        png_set_expand(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_expand(png_ptr);
    if (bit_depth_ == 16)
        png_set_strip_16(png_ptr);
    if (color_type_ == PNG_COLOR_TYPE_GRAY ||
        color_type_ == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    // quick hack -- only work in >=libpng 1.2.7
    png_set_add_alpha(png_ptr,0xff,PNG_FILLER_AFTER); //rgba

    double gamma;
    if (png_get_gAMA(png_ptr, info_ptr, &gamma))
        png_set_gamma(png_ptr, 2.2, gamma);

    if (x0 == 0 && y0 == 0 && image.width() >= width_ && image.height() >= height_)
    {

        if (png_get_interlace_type(png_ptr,info_ptr) == PNG_INTERLACE_ADAM7)
        {
            png_set_interlace_handling(png_ptr); // FIXME: libpng bug?
            // according to docs png_read_image
            // "..automatically handles interlacing,
            // so you don't need to call png_set_interlace_handling()"
        }
        png_read_update_info(png_ptr, info_ptr);
        // we can read whole image at once
        // alloc row pointers
        boost::scoped_array<png_byte*> rows(new png_bytep[height_]);
        for (unsigned i=0; i<height_; ++i)
            rows[i] = (png_bytep)image.getRow(i);
        png_read_image(png_ptr, rows.get());
    }
    else
    {
        png_read_update_info(png_ptr, info_ptr);
        unsigned w=std::min(unsigned(image.width()),width_);
        unsigned h=std::min(unsigned(image.height()),height_);
        unsigned rowbytes=png_get_rowbytes(png_ptr, info_ptr);
        boost::scoped_array<png_byte> row(new png_byte[rowbytes]);
        //START read image rows
        for (unsigned i=0;i<height_;++i)
        {
            png_read_row(png_ptr,row.get(),0);
            if (i>=y0 && i<h)
            {
                image.setRow(i-y0,reinterpret_cast<unsigned*>(&row[x0]),w);
            }
        }
        //END
    }

    png_read_end(png_ptr,0);
    png_destroy_read_struct(&png_ptr, &info_ptr,0);
    fclose(fp);
}
}
