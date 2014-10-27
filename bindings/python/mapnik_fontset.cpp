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

#include <mapnik/config.hpp>

// boost
#include "boost_std_shared_shim.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#pragma GCC diagnostic pop

//mapnik
#include <mapnik/font_set.hpp>


using mapnik::font_set;

void export_fontset ()
{
    using namespace boost::python;
    class_<font_set>("FontSet", init<std::string const&>("default fontset constructor")
        )
        .add_property("name",
                       make_function(&font_set::get_name,return_value_policy<copy_const_reference>()),
                       &font_set::set_name,
                      "Get/Set the name of the FontSet.\n"
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
