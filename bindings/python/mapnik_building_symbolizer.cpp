/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko, Jean-Francois Doyon
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

#include <boost/python.hpp>
#include <mapnik/building_symbolizer.hpp>

using namespace mapnik;
using mapnik::building_symbolizer;
using mapnik::color;

void export_building_symbolizer()
{
    using namespace boost::python;

    class_<building_symbolizer>("BuildingSymbolizer",
                               init<>("Default BuildingSymbolizer"))
        .add_property("fill",make_function
                      (&building_symbolizer::get_fill,
                       return_value_policy<copy_const_reference>()),
                      &building_symbolizer::set_fill)
        .add_property("fill_opacity",
                      &building_symbolizer::get_opacity,
                      &building_symbolizer::set_opacity)
        .add_property("height",
                      make_function(&building_symbolizer::height,
                                    return_value_policy<copy_const_reference>()),
                      &building_symbolizer::set_height,
                      "Set/get the building height")
        ;

}
