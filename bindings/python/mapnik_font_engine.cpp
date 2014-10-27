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
#include <boost/noncopyable.hpp>
#pragma GCC diagnostic pop

#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/utils.hpp>

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
        .def("register_fonts",&freetype_engine::register_fonts)
        .def("face_names",&freetype_engine::face_names)
        .staticmethod("register_font")
        .staticmethod("register_fonts")
        .staticmethod("face_names")
        ;
}
