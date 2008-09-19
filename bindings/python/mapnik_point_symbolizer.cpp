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
#include <mapnik/point_symbolizer.hpp>

void export_point_symbolizer()
{
    using namespace boost::python;
    using mapnik::point_symbolizer;
    
    class_<point_symbolizer>("PointSymbolizer",
                             init<>("Default Point Symbolizer - 4x4 black square"))
        .def (init<std::string const&,
              std::string const&,unsigned,unsigned>("TODO"))
        .add_property("allow_overlap",
              &point_symbolizer::get_allow_overlap,
              &point_symbolizer::set_allow_overlap)
        .add_property("opacity",
              &point_symbolizer::get_opacity,
              &point_symbolizer::set_opacity)
        ;
}
