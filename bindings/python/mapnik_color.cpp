/* This file is part of python_mapnik (c++/python mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko, Jean-Francois Doyon
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: mapnik_color.cc 17 2005-03-08 23:58:43Z pavlenko $



#include <boost/python.hpp>
#include <mapnik.hpp>

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

void export_color () 
{
    using namespace boost::python;
    class_<Color>("Color",init<>())
        .def(init<int,int,int,boost::python::optional<int> >())
        .add_property("r",&Color::red,&Color::set_red)
        .add_property("g",&Color::green,&Color::set_green)
        .add_property("b",&Color::blue,&Color::set_blue)
	.add_property("a",&Color::alpha)
	.def(self == self)
        .def_pickle(color_pickle_suite())
        ;
    def("Color",&create_from_string);
}

