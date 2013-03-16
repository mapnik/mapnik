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

// jpeg
extern "C"
{
#include <jpeglib.h>
}

// boost
#include <boost/scoped_array.hpp>
#include <boost/utility.hpp>

// std
#include <cstdio>

namespace mapnik
{
class JpegReader : public image_reader, boost::noncopyable
{
private:
    std::string fileName_;
    unsigned width_;
    unsigned height_;
public:
    explicit JpegReader(std::string const& fileName);
    ~JpegReader();
    unsigned width() const;
    unsigned height() const;
    inline bool premultiplied_alpha() const { return true; }
    void read(unsigned x,unsigned y,image_data_32& image);
private:
    void init();
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
      height_(0)
{
    init();
}

JpegReader::~JpegReader() {}

void JpegReader::init()
{
    FILE *fp = fopen(fileName_.c_str(),"rb");
    if (!fp) throw image_reader_exception("JPEG Reader: cannot open image file " + fileName_);

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);

    jpeg_start_decompress(&cinfo);
    width_ = cinfo.output_width;
    height_ = cinfo.output_height;
    // if enabled: "Application transferred too few scanlines"
    //jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
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
    struct jpeg_decompress_struct cinfo;

    FILE *fp = fopen(fileName_.c_str(),"rb");
    if (!fp) throw image_reader_exception("JPEG Reader: cannot open image file " + fileName_);

    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);

    jpeg_read_header(&cinfo, TRUE);
    if (cinfo.out_color_space == JCS_UNKNOWN)
        throw image_reader_exception("JPEG Reader: failed to read unknown color space in " + fileName_);

    jpeg_start_decompress(&cinfo);

    if (cinfo.output_width == 0) {
        jpeg_destroy_decompress (&cinfo);
        fclose(fp);
        throw image_reader_exception("JPEG Reader: failed to read image size of " + fileName_);
    }

    JSAMPARRAY buffer;
    int row_stride;
    unsigned char a,r,g,b;
    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    unsigned w = std::min(unsigned(image.width()),width_);
    unsigned h = std::min(unsigned(image.height()),height_);

    boost::scoped_array<unsigned int> out_row(new unsigned int[w]);
    // TODO - handle x0
    for (unsigned i=0;i<h;++i)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        if (i>=y0 && i<h)
        {
            for (unsigned int x=0; x<w; x++)
            {
                a = 255; // alpha not supported in jpg
                r = buffer[0][cinfo.output_components * x];
                if (cinfo.output_components > 2)
                {
                    g = buffer[0][cinfo.output_components*x+1];
                    b = buffer[0][cinfo.output_components*x+2];
                } else {
                    g = r;
                    b = r;
                }
                out_row[x] = color(r, g, b, a).rgba();
            }
            image.setRow(i-y0, out_row.get(), w);
        }
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
}
}
