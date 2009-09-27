/*****************************************************************************
 * 
 * This file is part of Mapnik (c+mapping toolkit)
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

// boost
#include <boost/python.hpp>

// mapnik
#include <mapnik/stroke.hpp>
#include "mapnik_enumeration.hpp"

using namespace mapnik;

namespace {
  using namespace boost::python;

  list get_dashes_list(const stroke& stroke)
  {
    list l;

    if (stroke.has_dash()) {
      mapnik::dash_array const& dash = stroke.get_dash_array();
      mapnik::dash_array::const_iterator iter = dash.begin();
      mapnik::dash_array::const_iterator end = dash.end();
      for (; iter != end; ++iter) {
          l.append(make_tuple(iter->first, iter->second));
      }
    }

    return l;
  }
}

struct stroke_pickle_suite : boost::python::pickle_suite
{
   static boost::python::tuple
   getinitargs(const stroke& s)
   {

      return boost::python::make_tuple(s.get_color(),s.get_width());
      
   }

   static  boost::python::tuple
   getstate(const stroke& s)
   {
        boost::python::list dashes = get_dashes_list(s);
        return boost::python::make_tuple(s.get_opacity(),dashes,s.get_line_cap(),s.get_line_join());
   }

   static void
   setstate (stroke& s, boost::python::tuple state)
   {
        using namespace boost::python;
        if (len(state) != 4)
        {
            PyErr_SetObject(PyExc_ValueError,
                         ("expected 4-item tuple in call to __setstate__; got %s"
                          % state).ptr()
            );
            throw_error_already_set();
        }

        s.set_opacity(extract<float>(state[0]));

        if (state[1])
        {
          list dashes = extract<list>(state[1]);
          for(boost::python::ssize_t i=0; i<len(dashes); i++) {
              double ds1 = extract<double>(dashes[i][0]);
              double ds2 = extract<double>(dashes[i][1]);
              s.add_dash(ds1,ds2);
          }
        }

        s.set_line_cap(extract<line_cap_e>(state[2]));

        s.set_line_join(extract<line_join_e>(state[3]));

   }

};


void export_stroke ()
{
    using namespace boost::python;

    enumeration_<line_cap_e>("line_cap",
        "The possible values for a line cap used when drawing\n"
        "with a stroke.\n")
        .value("BUTT_CAP",BUTT_CAP)
        .value("SQUARE_CAP",SQUARE_CAP)
        .value("ROUND_CAP",ROUND_CAP)
        ;
    enumeration_<line_join_e>("line_join",
        "The possible values for the line joining mode\n"
        "when drawing with a stroke.\n")
        .value("MITER_JOIN",MITER_JOIN)
        .value("MITER_REVERT_JOIN",MITER_REVERT_JOIN)
        .value("ROUND_JOIN",ROUND_JOIN)
        .value("BEVEL_JOIN",BEVEL_JOIN)
        ;

    class_<stroke>("Stroke",init<>(
        "Creates a new default black stroke with the width of 1.\n"))
        .def(init<color,float>(
              (arg("color"),arg("width")),
              "Creates a new stroke object with a specified color and width.\n")
        )
        .def_pickle(stroke_pickle_suite())
        .add_property("color",make_function
              (&stroke::get_color,return_value_policy<copy_const_reference>()),
              &stroke::set_color,
              "Gets or sets the stroke color.\n"
              "Returns a new Color object on retrieval.\n")
        .add_property("width",
              &stroke::get_width,
              &stroke::set_width,
              "Gets or sets the stroke width in pixels.\n") 
        .add_property("opacity",
              &stroke::get_opacity,
              &stroke::set_opacity, 
              "Gets or sets the opacity of this stroke.\n"
              "The value is a float between 0 and 1.\n")
        .add_property("line_cap",
              &stroke::get_line_cap,
              &stroke::set_line_cap,
              "Gets or sets the line cap of this stroke.\n")
        .add_property("line_join",
              &stroke::get_line_join,
              &stroke::set_line_join,
              "Returns the line join mode of this stroke.\n")
        // todo consider providing a single get/set property
        .def("add_dash",&stroke::add_dash,
              (arg("length"),arg("gap")),
              "Adds a dash segment to the dash patterns of this stroke.\n")
        .def("get_dashes", get_dashes_list,
              "Returns the list of dash segments for this stroke.\n")
        ;
}
