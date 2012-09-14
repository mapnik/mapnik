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


// boost
#include <boost/python.hpp>

//mapnik
#include <mapnik/font_set.hpp>


using mapnik::font_set;

void export_fontset ()
{
    using namespace boost::python;
    class_<font_set>("FontSet", init<>("default fontset constructor")
        )
        .def("add_face_name",&font_set::add_face_name,
             (arg("name")),
             "Add a face-name to the fontset.\n"
             "\n"
             "Example:\n"
             ">>> fs = Fontset('book-fonts')\n"
             ">>> fs.add_face_name('DejaVu Sans Book')\n")
        .add_property("names",make_function
                      (&font_set::get_face_names,
                       return_value_policy<reference_existing_object>()),
                      "List of face names belonging to a FontSet.\n"
            )
        ;
}
