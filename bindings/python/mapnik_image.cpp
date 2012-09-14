/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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

extern "C"
{
#include <png.h>
}

// boost
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/make_shared.hpp>

// mapnik
#include <mapnik/graphics.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/png_io.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/image_compositing.hpp>

// jpeg
#if defined(HAVE_JPEG)
#include <mapnik/jpeg_io.hpp>
#endif

// cairo
#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
#include <cairomm/surface.h>
#include <pycairo.h>
#endif

using mapnik::image_32;
using mapnik::image_reader;
using mapnik::get_image_reader;
using mapnik::type_from_filename;
using mapnik::save_to_file;
using mapnik::save_to_string;

using namespace boost::python;

// output 'raw' pixels
PyObject* tostring1( image_32 const& im)
{
    int size = im.width() * im.height() * 4;
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        ((const char*)im.raw_data(),size);
}

// encode (png,jpeg)
PyObject* tostring2(image_32 const & im, std::string const& format)
{
    std::string s = save_to_string(im, format);
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        (s.data(),s.size());
}

PyObject* tostring3(image_32 const & im, std::string const& format, mapnik::rgba_palette const& pal)
{
    std::string s = save_to_string(im, format, pal);
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        (s.data(),s.size());
}


void save_to_file1(mapnik::image_32 const& im, std::string const& filename)
{
    save_to_file(im,filename);
}

void save_to_file2(mapnik::image_32 const& im, std::string const& filename, std::string const& type)
{
    save_to_file(im,filename,type);
}

void save_to_file3(mapnik::image_32 const& im, std::string const& filename, std::string const& type, mapnik::rgba_palette const& pal)
{
    save_to_file(im,filename,type,pal);
}

bool painted(mapnik::image_32 const& im)
{
    return im.painted();
}

unsigned get_pixel(mapnik::image_32 const& im, int x, int y)
{
    if (x < static_cast<int>(im.width()) && y < static_cast<int>(im.height()))
    {
        mapnik::image_data_32 const & data = im.data();
        return data(x,y);
    }
    PyErr_SetString(PyExc_IndexError, "invalid x,y for image dimensions");
    boost::python::throw_error_already_set();
    return 0;
}

void set_pixel(mapnik::image_32 & im, unsigned x, unsigned y, mapnik::color const& c)
{
    im.setPixel(x, y, c.rgba());
}

boost::shared_ptr<image_32> open_from_file(std::string const& filename)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        std::auto_ptr<image_reader> reader(get_image_reader(filename,*type));
        if (reader.get())
        {

            boost::shared_ptr<image_32> image_ptr = boost::make_shared<image_32>(reader->width(),reader->height());
            reader->read(0,0,image_ptr->data());
            return image_ptr;
        }
        throw mapnik::image_reader_exception("Failed to load: " + filename);
    }
    throw mapnik::image_reader_exception("Unsupported image format:" + filename);
}

void blend (image_32 & im, unsigned x, unsigned y, image_32 const& im2, float opacity)
{
    im.set_rectangle_alpha2(im2.data(),x,y,opacity);
}


void composite(image_32 & dst, image_32 & src, mapnik::composite_mode_e mode, float opacity)
{
    mapnik::composite(dst.data(),src.data(),mode,opacity,0,0,false);
}

#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
boost::shared_ptr<image_32> from_cairo(PycairoSurface* surface)
{
    Cairo::RefPtr<Cairo::ImageSurface> s(new Cairo::ImageSurface(surface->surface));
    boost::shared_ptr<image_32> image_ptr = boost::make_shared<image_32>(s);
    return image_ptr;
}
#endif

void export_image()
{
    using namespace boost::python;
    enum_<mapnik::composite_mode_e>("CompositeOp")
        .value("clear", mapnik::clear)
        .value("src", mapnik::src)
        .value("dst", mapnik::dst)
        .value("src_over", mapnik::src_over)
        .value("dst_over", mapnik::dst_over)
        .value("src_in", mapnik::src_in)
        .value("dst_in", mapnik::dst_in)
        .value("src_out", mapnik::src_out)
        .value("dst_out", mapnik::dst_out)
        .value("src_atop", mapnik::src_atop)
        .value("dst_atop", mapnik::dst_atop)
        .value("xor", mapnik::_xor)
        .value("plus", mapnik::plus)
        .value("minus", mapnik::minus)
        .value("multiply", mapnik::multiply)
        .value("screen", mapnik::screen)
        .value("overlay", mapnik::overlay)
        .value("darken", mapnik::darken)
        .value("lighten", mapnik::lighten)
        .value("color_dodge", mapnik::color_dodge)
        .value("color_burn", mapnik::color_burn)
        .value("hard_light", mapnik::hard_light)
        .value("soft_light", mapnik::soft_light)
        .value("difference", mapnik::difference)
        .value("exclusion", mapnik::exclusion)
        .value("contrast", mapnik::contrast)
        .value("invert", mapnik::invert)
        .value("invert_rgb", mapnik::invert_rgb)
        ;

    class_<image_32,boost::shared_ptr<image_32> >("Image","This class represents a 32 bit RGBA image.",init<int,int>())
        .def("width",&image_32::width)
        .def("height",&image_32::height)
        .def("view",&image_32::get_view)
        .def("painted",&painted)
        .add_property("background",make_function
                      (&image_32::get_background,return_value_policy<copy_const_reference>()),
                      &image_32::set_background, "The background color of the image.")
        .def("set_grayscale_to_alpha",&image_32::set_grayscale_to_alpha, "Set the grayscale values to the alpha channel of the Image")
        .def("set_color_to_alpha",&image_32::set_color_to_alpha, "Set a given color to the alpha channel of the Image")
        .def("set_alpha",&image_32::set_alpha, "Set the overall alpha channel of the Image")
        .def("blend",&blend)
        .def("composite",&composite,
         ( arg("self"),
           arg("image"),
           arg("mode"),
           arg("opacity")=1.0f
         ))
        .def("premultiply",&image_32::premultiply)
        .def("demultiply",&image_32::demultiply)
        .def("set_pixel",&set_pixel)
        .def("get_pixel",&get_pixel)
        //TODO(haoyu) The method name 'tostring' might be confusing since they actually return bytes in Python 3

        .def("tostring",&tostring1)
        .def("tostring",&tostring2)
        .def("tostring",&tostring3)
        .def("save", &save_to_file1)
        .def("save", &save_to_file2)
        .def("save", &save_to_file3)
        .def("open",open_from_file)
        .staticmethod("open")
#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
        .def("from_cairo",&from_cairo)
        .staticmethod("from_cairo")
#endif
        ;

}
