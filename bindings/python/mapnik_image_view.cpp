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
#include <mapnik/image_data.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/palette.hpp>
#include <mapnik/image_view.hpp>
#include <sstream>

using mapnik::image_data_32;
using mapnik::image_view;
using mapnik::save_to_file;

// output 'raw' pixels
PyObject* view_tostring1(image_view<image_data_32> const& view)
{
    std::ostringstream ss(std::ios::out|std::ios::binary);
    for (unsigned i=0;i<view.height();i++)
    {
        ss.write(reinterpret_cast<const char*>(view.getRow(i)),
                 view.width() * sizeof(image_view<image_data_32>::pixel_type));
    }
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        ((const char*)ss.str().c_str(),ss.str().size());
}

// encode (png,jpeg)
PyObject* view_tostring2(image_view<image_data_32> const & view, std::string const& format)
{
    std::string s = save_to_string(view, format);
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        (s.data(),s.size());
}

PyObject* view_tostring3(image_view<image_data_32> const & view, std::string const& format, mapnik::rgba_palette const& pal)
{
    std::string s = save_to_string(view, format, pal);
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        (s.data(),s.size());
}

bool is_solid(image_view<image_data_32> const& view)
{
    if (view.width() > 0 && view.height() > 0)
    {
        mapnik::image_view<image_data_32>::pixel_type const* first_row = view.getRow(0);
        mapnik::image_view<image_data_32>::pixel_type const first_pixel = first_row[0];
        for (unsigned y = 0; y < view.height(); ++y)
        {
            mapnik::image_view<image_data_32>::pixel_type const * row = view.getRow(y);
            for (unsigned x = 0; x < view.width(); ++x)
            {
                if (first_pixel != row[x])
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void save_view1(image_view<image_data_32> const& view,
                std::string const& filename)
{
    save_to_file(view,filename);
}

void save_view2(image_view<image_data_32> const& view,
                std::string const& filename,
                std::string const& type)
{
    save_to_file(view,filename,type);
}

void save_view3(image_view<image_data_32> const& view,
                std::string const& filename,
                std::string const& type,
                mapnik::rgba_palette const& pal)
{
    save_to_file(view,filename,type,pal);
}


void export_image_view()
{
    using namespace boost::python;
    class_<image_view<image_data_32> >("ImageView","A view into an image.",no_init)
        .def("width",&image_view<image_data_32>::width)
        .def("height",&image_view<image_data_32>::height)
        .def("is_solid",&is_solid)
        .def("tostring",&view_tostring1)
        .def("tostring",&view_tostring2)
        .def("tostring",&view_tostring3)
        .def("save",&save_view1)
        .def("save",&save_view2)
        .def("save",&save_view3)
        ;
}
