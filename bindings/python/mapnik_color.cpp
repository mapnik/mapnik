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

// boost
#include <boost/python.hpp>

//mapnik
#include <mapnik/color.hpp>


using mapnik::color;

struct color_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const color& c)
    {
        using namespace boost::python;
        return boost::python::make_tuple(c.red(),c.green(),c.blue(),c.alpha());
    }
};

void export_color ()
{
    using namespace boost::python;
    class_<color>("Color", init<int,int,int,int>(
                      ( arg("r"), arg("g"), arg("b"), arg("a") ),
                      "Creates a new color from its RGB components\n"
                      "and an alpha value.\n"
                      "All values between 0 and 255.\n")
        )
        .def(init<int,int,int>(
                 ( arg("r"), arg("g"), arg("b") ),
                 "Creates a new color from its RGB components.\n"
                 "All values between 0 and 255.\n")
            )
        .def(init<std::string>(
                 ( arg("color_string") ),
                 "Creates a new color from its CSS string representation.\n"
                 "The string may be a CSS color name (e.g. 'blue')\n"
                 "or a hex color string (e.g. '#0000ff').\n")
            )
        .add_property("r",
                      &color::red,
                      &color::set_red,
                      "Gets or sets the red component.\n"
                      "The value is between 0 and 255.\n")
        .add_property("g",
                      &color::green,
                      &color::set_green,
                      "Gets or sets the green component.\n"
                      "The value is between 0 and 255.\n")
        .add_property("b",
                      &color::blue,
                      &color::set_blue,
                      "Gets or sets the blue component.\n"
                      "The value is between 0 and 255.\n")
        .add_property("a",
                      &color::alpha,
                      &color::set_alpha,
                      "Gets or sets the alpha component.\n"
                      "The value is between 0 and 255.\n")
        .def(self == self)
        .def(self != self)
        .def_pickle(color_pickle_suite())
        .def("__str__",&color::to_string)
        .def("to_hex_string",&color::to_hex_string,
             "Returns the hexadecimal representation of this color.\n"
             "\n"
             "Example:\n"
             ">>> c = Color('blue')\n"
             ">>> c.to_hex_string()\n"
             "'#0000ff'\n")
        ;
}
