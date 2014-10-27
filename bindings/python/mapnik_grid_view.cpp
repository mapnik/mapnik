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

#if defined(GRID_RENDERER)

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
#include <string>
#include <mapnik/grid/grid_view.hpp>
#include <mapnik/grid/grid.hpp>
#include "python_grid_utils.hpp"

using namespace boost::python;

// help compiler see template definitions
static dict (*encode)( mapnik::grid_view const&, std::string const& , bool, unsigned int) = mapnik::grid_encode;

void export_grid_view()
{
    class_<mapnik::grid_view,
        std::shared_ptr<mapnik::grid_view> >("GridView",
                                               "This class represents a feature hitgrid subset.",no_init)
        .def("width",&mapnik::grid_view::width)
        .def("height",&mapnik::grid_view::height)
        .def("encode",encode,
             ( boost::python::arg("encoding")="utf",boost::python::arg("add_features")=true,boost::python::arg("resolution")=4 ),
             "Encode the grid as as optimized json\n"
            )
        ;
}

#endif
