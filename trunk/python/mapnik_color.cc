/* This file is part of python_mapnik (c++/python mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
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

//$Id$


#include <mapnik.hh>
#include <boost/python.hpp>

using mapnik::Color;

struct color_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
        getinitargs(const Color& c)
    {
        using namespace boost::python;
        return make_tuple(c.red(),c.green(),c.blue());
    }
};

void export_color () 
{
    using namespace boost::python;
    class_<Color>("color",init<>())
        .def(init<int,int,int>())
        .def("red",&Color::red)
        .def("green",&Color::green)
        .def("blue",&Color::blue)
        .def_pickle(color_pickle_suite())
        ;
}



