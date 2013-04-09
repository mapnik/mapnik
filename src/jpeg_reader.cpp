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

// mapnik
#include <mapnik/image_reader.hpp>
#include <mapnik/color.hpp>
#include <mapnik/noncopyable.hpp>

// jpeg
extern "C"
{
#include <jpeglib.h>
}

// boost
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>

// std
#include <cstdio>

namespace mapnik
{
class JpegReader : public image_reader, mapnik::noncopyable
{
    struct file_closer
    {
       void operator() (FILE * file)
       {
           if (file != 0) fclose(file);
       }
    };
    typedef boost::shared_ptr<FILE> file_ptr;

private:
    std::string fileName_;
    unsigned width_;
    unsigned height_;
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;
    file_ptr file_;
public:
    explicit JpegReader(std::string const& fileName);
    ~JpegReader();
    unsigned width() const;
    unsigned height() const;
    inline bool premultiplied_alpha() const { return true; }
    void read(unsigned x,unsigned y,image_data_32& image);
private:
    void init();
    static void on_error(j_common_ptr cinfo);
    static void on_error_message(j_common_ptr cinfo);
};

namespace
{
image_reader* createJpegReader(std::string const& file)
{
    return new JpegReader(file);
}
const bool registered = register_image_reader("jpeg",createJpegReader);
}

JpegReader::JpegReader(std::string const& fileName)
    : fileName_(fileName),
      width_(0),
      height_(0),
      file_()
{
    init();
}

JpegReader::~JpegReader()
{
    jpeg_destroy_decompress(&cinfo);
}

void JpegReader::on_error(j_common_ptr cinfo)
{
    (*cinfo->err->output_message)(cinfo);
    jpeg_destroy(cinfo);
    throw image_reader_exception("JPEG Reader: libjpeg could not read image");
}

void JpegReader::on_error_message(j_common_ptr cinfo)
{
    // used to supress jpeg from printing to stderr
}

void JpegReader::init()
{
    FILE * fp = fopen(fileName_.c_str(),"rb");
    if (!fp) throw image_reader_exception("JPEG Reader: cannot open image file " + fileName_);
    file_ = boost::shared_ptr<FILE>(fp,file_closer());
    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = on_error;
    jerr.output_message = on_error_message;
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, &*file_);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    width_ = cinfo.output_width;
    height_ = cinfo.output_height;

    if (cinfo.out_color_space == JCS_UNKNOWN)
    {
        throw image_reader_exception("JPEG Reader: failed to read unknown color space in " + fileName_);
    }
    if (cinfo.output_width == 0 || cinfo.output_height == 0)
    {
        jpeg_destroy_decompress (&cinfo);
        throw image_reader_exception("JPEG Reader: failed to read image size of " + fileName_);
    }
}

unsigned JpegReader::width() const
{
    return width_;
}

unsigned JpegReader::height() const
{
    return height_;
}

void JpegReader::read(unsigned x0, unsigned y0, image_data_32& image)
{
    JSAMPARRAY buffer;
    int row_stride;
    unsigned char a,r,g,b;
    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    unsigned w = std::min(unsigned(image.width()),width_ - x0);
    unsigned h = std::min(unsigned(image.height()),height_ - y0);

    boost::scoped_array<unsigned int> out_row(new unsigned int[w]);
    unsigned row = 0;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        if (row >= y0 && row < y0 + h)
        {
            for (unsigned int x = 0; x < w; ++x)
            {
                unsigned col = x + x0;
                a = 255; // alpha not supported in jpg
                r = buffer[0][cinfo.output_components * col];
                if (cinfo.output_components > 2)
                {
                    g = buffer[0][cinfo.output_components * col + 1];
                    b = buffer[0][cinfo.output_components * col + 2];
                } else {
                    g = r;
                    b = r;
                }
                out_row[x] = color(r, g, b, a).rgba();
            }
            image.setRow(row - y0, out_row.get(), w);
        }
        ++row;
    }
    jpeg_destroy_decompress(&cinfo);
}

}
