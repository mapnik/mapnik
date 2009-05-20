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
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <mapnik/feature_type_style.hpp>

using mapnik::feature_type_style;
using mapnik::rules;
using mapnik::rule_type;

struct style_pickle_suite : boost::python::pickle_suite
{
   static boost::python::tuple
   getstate(const feature_type_style& s)
   {
        boost::python::list rule_list;

        rules::const_iterator it = s.get_rules().begin();
        rules::const_iterator end = s.get_rules().end();
        for (; it != end; ++it)
        {
            rule_list.append( *it );    
        }

      return boost::python::make_tuple(rule_list);
   }

   static void
   setstate (feature_type_style& s, boost::python::tuple state)
   {
        using namespace boost::python;
        if (len(state) != 1)
        {
            PyErr_SetObject(PyExc_ValueError,
                         ("expected 1-item tuple in call to __setstate__; got %s"
                          % state).ptr()
            );
            throw_error_already_set();
        }
        
        boost::python::list rules = extract<boost::python::list>(state[0]);
        for (int i=0; i<len(rules); ++i)
        {
            s.add_rule(extract<rule_type>(rules[i]));
        }
   }
   
};

void export_style()
{
    using namespace boost::python;

    class_<rules>("Rules",init<>("default ctor"))
        .def(vector_indexing_suite<rules>())
        ;
    class_<feature_type_style>("Style",init<>("default style constructor"))

        .def_pickle(style_pickle_suite()
           )

        .add_property("rules",make_function
                      (&feature_type_style::get_rules,
                       return_value_policy<reference_existing_object>()))
        ;
    
}

