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
#include <mapnik/line_symbolizer.hpp>

using mapnik::line_symbolizer;
using mapnik::stroke;
using mapnik::color;

struct line_symbolizer_pickle_suite : boost::python::pickle_suite
{
   static boost::python::tuple
   getinitargs(const line_symbolizer& l)
   {
      return boost::python::make_tuple(l.get_stroke());
   }

};

void export_line_symbolizer()
{
    using namespace boost::python;
    
    class_<line_symbolizer>("LineSymbolizer",
                            init<>("Default LineSymbolizer - 1px solid black"))
        .def(init<stroke const&>("TODO"))
        .def(init<color const& ,float>())
        .def_pickle(line_symbolizer_pickle_suite())
        .add_property("stroke",make_function
                      (&line_symbolizer::get_stroke,
                       return_value_policy<copy_const_reference>()),
                      &line_symbolizer::set_stroke)
	;    
}
