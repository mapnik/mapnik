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
//$Id$


#include <boost/python.hpp>
#include <color.hpp>
#include <color_factory.hpp>

using mapnik::Color;
using mapnik::color_factory;

struct color_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const Color& c)
    {
        using namespace boost::python;
        return boost::python::make_tuple(c.red(),c.green(),c.blue());
    }
};

Color create_from_string(const char* str)
{
    return color_factory::from_string(str);
}

Color create_from_rgb(unsigned r, unsigned g,unsigned b)
{
    return Color(r,g,b);
}

Color create_from_rgba(unsigned r, unsigned g,unsigned b,unsigned a)
{
    return Color(r,g,b,a);
}

void export_color () 
{
    using namespace boost::python;
    class_<Color>("Color",init<>())
        .add_property("r",&Color::red,&Color::set_red)
        .add_property("g",&Color::green,&Color::set_green)
        .add_property("b",&Color::blue,&Color::set_blue)
        .add_property("a",&Color::alpha,&Color::set_alpha)
        .def(self == self)
        .def_pickle(color_pickle_suite())
        .def("__str__",&Color::to_string)
        ;
    
    def("Color",&create_from_string);
    def("Color",&create_from_rgba);
    def("Color",&create_from_rgb);
}

