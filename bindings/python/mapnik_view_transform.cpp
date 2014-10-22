/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko, Jean-Francois Doyon
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/view_transform.hpp>

using mapnik::view_transform;

struct view_transform_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const view_transform& c)
    {
        using namespace boost::python;
        return boost::python::make_tuple(c.width(),c.height(),c.extent());
    }
};

namespace {

mapnik::coord2d forward_point(mapnik::view_transform const& t, mapnik::coord2d const& in)
{
    mapnik::coord2d out(in);
    t.forward(out);
    return out;
}

mapnik::coord2d backward_point(mapnik::view_transform const& t, mapnik::coord2d const& in)
{
    mapnik::coord2d out(in);
    t.backward(out);
    return out;
}

mapnik::box2d<double> forward_envelope(mapnik::view_transform const& t, mapnik::box2d<double> const& in)
{
    return t.forward(in);
}

mapnik::box2d<double> backward_envelope(mapnik::view_transform const& t, mapnik::box2d<double> const& in)
{
    return t.backward(in);
}
}

void export_view_transform()
{
    using namespace boost::python;
    using mapnik::box2d;
    using mapnik::coord2d;

    class_<view_transform>("ViewTransform",init<int,int,box2d<double> const& > (
                               "Create a ViewTransform with a width and height as integers and extent"))
        .def_pickle(view_transform_pickle_suite())
        .def("forward", forward_point)
        .def("backward",backward_point)
        .def("forward", forward_envelope)
        .def("backward",backward_envelope)
        .def("scale_x",&view_transform::scale_x)
        .def("scale_y",&view_transform::scale_y)
        ;
}
