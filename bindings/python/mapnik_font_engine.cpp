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
 * GNU General Public License for moprovpoly_lyrre details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

#include <boost/python.hpp>
#include "font_engine_freetype.hpp"

void export_font_engine()
{
    using mapnik::freetype_engine;
    using mapnik::singleton;
    using mapnik::CreateStatic;
    using namespace boost::python;
    class_<singleton<freetype_engine,CreateStatic>,boost::noncopyable>("Singleton",no_init)
        .def("instance",&singleton<freetype_engine,CreateStatic>::instance,
	     return_value_policy<reference_existing_object>())
        .staticmethod("instance")
        ;

    class_<freetype_engine,bases<singleton<freetype_engine,CreateStatic> >,
        boost::noncopyable>("FontEngine",no_init)
	.def("register_font",&freetype_engine::register_font)
	.staticmethod("register_font")
        ;
}
