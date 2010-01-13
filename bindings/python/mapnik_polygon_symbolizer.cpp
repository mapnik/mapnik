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
#include <mapnik/polygon_symbolizer.hpp>

using mapnik::polygon_symbolizer;
using mapnik::color;

struct polygon_symbolizer_pickle_suite : boost::python::pickle_suite
{
   static boost::python::tuple
   getinitargs(const polygon_symbolizer& p)
   {
      return boost::python::make_tuple(p.get_fill());  
   }

   static  boost::python::tuple
   getstate(const polygon_symbolizer& p)
   {
        return boost::python::make_tuple(p.get_opacity(),p.get_gamma());
   }

   static void
   setstate (polygon_symbolizer& p, boost::python::tuple state)
   {
        using namespace boost::python;
        if (len(state) != 2)
        {
            PyErr_SetObject(PyExc_ValueError,
                         ("expected 2-item tuple in call to __setstate__; got %s"
                          % state).ptr()
            );
            throw_error_already_set();
        }
                
        p.set_opacity(extract<float>(state[0]));
        p.set_gamma(extract<float>(state[1]));
   }

};

void export_polygon_symbolizer()
{
    using namespace boost::python;
        
    class_<polygon_symbolizer>("PolygonSymbolizer",
				    init<>("Default PolygonSymbolizer - solid fill grey"))
        .def(init<color const&>("TODO"))
        .def_pickle(polygon_symbolizer_pickle_suite())
        .add_property("fill",make_function
                      (&polygon_symbolizer::get_fill,
                       return_value_policy<copy_const_reference>()),
                      &polygon_symbolizer::set_fill)
        .add_property("fill_opacity",
                      &polygon_symbolizer::get_opacity,
                      &polygon_symbolizer::set_opacity)
        .add_property("gamma",
                      &polygon_symbolizer::get_gamma,
                      &polygon_symbolizer::set_gamma)
        ;    

}
