/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko, Jean-Francois Doyon
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

#include <mapnik/config.hpp>

// boost
#include "boost_std_shared_shim.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_view_any.hpp>

// cairo
#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
#include <mapnik/cairo/cairo_context.hpp>
#include <mapnik/cairo/cairo_image_util.hpp>
#include <pycairo.h>
#include <cairo.h>
#endif

using mapnik::image_any;
using mapnik::image_reader;
using mapnik::get_image_reader;
using mapnik::type_from_filename;
using mapnik::save_to_file;

using namespace boost::python;

// output 'raw' pixels
PyObject* tostring1( image_any const& im)
{
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        ((const char*)im.getBytes(),im.getSize());
}

// encode (png,jpeg)
PyObject* tostring2(image_any const & im, std::string const& format)
{
    std::string s = mapnik::save_to_string(im, format);
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        (s.data(),s.size());
}

PyObject* tostring3(image_any const & im, std::string const& format, mapnik::rgba_palette const& pal)
{
    std::string s = mapnik::save_to_string(im, format, pal);
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        (s.data(),s.size());
}


void save_to_file1(mapnik::image_any const& im, std::string const& filename)
{
    save_to_file(im,filename);
}

void save_to_file2(mapnik::image_any const& im, std::string const& filename, std::string const& type)
{
    save_to_file(im,filename,type);
}

void save_to_file3(mapnik::image_any const& im, std::string const& filename, std::string const& type, mapnik::rgba_palette const& pal)
{
    save_to_file(im,filename,type,pal);
}

mapnik::image_view_any get_view(mapnik::image_any const& data,unsigned x,unsigned y, unsigned w,unsigned h)
{
    return mapnik::create_view(data,x,y,w,h);
}

bool painted(mapnik::image_any const& im)
{
    return im.painted();
}

bool is_solid(mapnik::image_any const& im)
{
    return mapnik::is_solid(im);
}

void background(mapnik::image_any & im, mapnik::color const& c)
{
    mapnik::fill(im, c);
}

uint32_t get_pixel(mapnik::image_any const& im, int x, int y)
{
    if (x < static_cast<int>(im.width()) && y < static_cast<int>(im.height()))
    {
        return mapnik::get_pixel<mapnik::image_any, uint32_t>(im, x, y);
    }
    PyErr_SetString(PyExc_IndexError, "invalid x,y for image dimensions");
    boost::python::throw_error_already_set();
    return 0;
}

void set_pixel(mapnik::image_any & im, unsigned x, unsigned y, mapnik::color const& c)
{
    mapnik::set_pixel(im, x, y, c);
}

std::shared_ptr<image_any> open_from_file(std::string const& filename)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        std::unique_ptr<image_reader> reader(get_image_reader(filename,*type));
        if (reader.get())
        {
            return std::make_shared<image_any>(std::move(reader->read(0,0,reader->width(),reader->height())));
        }
        throw mapnik::image_reader_exception("Failed to load: " + filename);
    }
    throw mapnik::image_reader_exception("Unsupported image format:" + filename);
}

std::shared_ptr<image_any> fromstring(std::string const& str)
{
    std::unique_ptr<image_reader> reader(get_image_reader(str.c_str(),str.size()));
    if (reader.get())
    {
        return std::make_shared<image_any>(std::move(reader->read(0,0,reader->width(), reader->height())));
    }
    throw mapnik::image_reader_exception("Failed to load image from buffer" );
}

std::shared_ptr<image_any> frombuffer(PyObject * obj)
{
    void const* buffer=0;
    Py_ssize_t buffer_len;
    if (PyObject_AsReadBuffer(obj, &buffer, &buffer_len) == 0)
    {
        std::unique_ptr<image_reader> reader(get_image_reader(reinterpret_cast<char const*>(buffer),buffer_len));
        if (reader.get())
        {
            return std::make_shared<image_any>(reader->read(0,0,reader->width(),reader->height()));
        }
    }
    throw mapnik::image_reader_exception("Failed to load image from buffer" );
}

void set_grayscale_to_alpha(image_any & im)
{
    mapnik::set_grayscale_to_alpha(im);
}

void set_color_to_alpha(image_any & im, mapnik::color const& c)
{
    mapnik::set_color_to_alpha(im, c);
}

void set_alpha(image_any & im, float opacity)
{
    mapnik::set_alpha(im, opacity);
}

bool premultiplied(image_any &im)
{
    return im.get_premultiplied();
}

bool premultiply(image_any & im)
{
    return mapnik::premultiply_alpha(im);
}

bool demultiply(image_any & im)
{
    return mapnik::demultiply_alpha(im);
}

void clear(image_any & im)
{
    mapnik::fill(im, 0);
}

void composite(image_any & dst, image_any & src, mapnik::composite_mode_e mode, float opacity, int dx, int dy)
{
    bool demultiply_dst = mapnik::premultiply_alpha(dst);
    bool demultiply_src = mapnik::premultiply_alpha(src);
    mapnik::composite(dst,src,mode,opacity,dx,dy);
    if (demultiply_dst) 
    {
        mapnik::demultiply_alpha(dst);
    }
    if (demultiply_src)
    {
        mapnik::demultiply_alpha(src);
    }
}

#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
std::shared_ptr<image_any> from_cairo(PycairoSurface* py_surface)
{
    mapnik::cairo_surface_ptr surface(cairo_surface_reference(py_surface->surface), mapnik::cairo_surface_closer());
    mapnik::image_rgba8 image = mapnik::image_rgba8(cairo_image_surface_get_width(&*surface), cairo_image_surface_get_height(&*surface));
    cairo_image_to_rgba8(image, surface);
    return std::make_shared<image_any>(std::move(image));
}
#endif

void export_image()
{
    using namespace boost::python;
    // NOTE: must match list in include/mapnik/image_compositing.hpp
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
        .value("grain_merge", mapnik::grain_merge)
        .value("grain_extract", mapnik::grain_extract)
        .value("hue", mapnik::hue)
        .value("saturation", mapnik::saturation)
        .value("color", mapnik::_color)
        .value("value", mapnik::_value)
        .value("linear_dodge", mapnik::linear_dodge)
        .value("linear_burn", mapnik::linear_burn)
        .value("divide", mapnik::divide)
        ;

    enum_<mapnik::image_dtype>("ImageType")
        .value("rgba8", mapnik::image_dtype_rgba8)
        .value("gray8", mapnik::image_dtype_gray8)
        .value("gray16", mapnik::image_dtype_gray16)
        .value("gray32f", mapnik::image_dtype_gray32f)
        ;

    class_<image_any,std::shared_ptr<image_any>, boost::noncopyable >("Image","This class represents a image.",init<int,int>())
        .def(init<int,int,mapnik::image_dtype>())
        .def(init<int,int,mapnik::image_dtype,bool>())
        .def(init<int,int,mapnik::image_dtype,bool,bool>())
        .def(init<int,int,mapnik::image_dtype,bool,bool,bool>())
        .def("width",&image_any::width)
        .def("height",&image_any::height)
        .def("view",&get_view)
        .def("painted",&painted)
        .def("is_solid",&is_solid)
        .def("background",&background, "Set the background color of the image.")
        .def("set_grayscale_to_alpha",&set_grayscale_to_alpha, "Set the grayscale values to the alpha channel of the Image")
        .def("set_color_to_alpha",&set_color_to_alpha, "Set a given color to the alpha channel of the Image")
        .def("set_alpha",&set_alpha, "Set the overall alpha channel of the Image")
        .def("composite",&composite,
         ( arg("self"),
           arg("image"),
           arg("mode")=mapnik::src_over,
           arg("opacity")=1.0f,
           arg("dx")=0,
           arg("dy")=0
         ))
        .def("premultiplied",&premultiplied)
        .def("premultiply",&premultiply)
        .def("demultiply",&demultiply)
        .def("set_pixel",&set_pixel)
        .def("get_pixel",&get_pixel)
        .def("clear",&clear)
        //TODO(haoyu) The method name 'tostring' might be confusing since they actually return bytes in Python 3

        .def("tostring",&tostring1)
        .def("tostring",&tostring2)
        .def("tostring",&tostring3)
        .def("save", &save_to_file1)
        .def("save", &save_to_file2)
        .def("save", &save_to_file3)
        .def("open",open_from_file)
        .staticmethod("open")
        .def("frombuffer",&frombuffer)
        .staticmethod("frombuffer")
        .def("fromstring",&fromstring)
        .staticmethod("fromstring")
#if defined(HAVE_CAIRO) && defined(HAVE_PYCAIRO)
        .def("from_cairo",&from_cairo)
        .staticmethod("from_cairo")
#endif
        ;

}
