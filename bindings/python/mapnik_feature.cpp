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

// boost
#include <boost/python/suite/indexing/indexing_suite.hpp>
#include <boost/python/iterator.hpp>
#include <boost/python/call_method.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python.hpp>
#include <boost/scoped_array.hpp>
// mapnik
#include <mapnik/feature.hpp>

mapnik::geometry2d & (mapnik::Feature::*get_geom1)(unsigned) = &mapnik::Feature::get_geometry;

namespace boost { namespace python {
      struct value_converter : public boost::static_visitor<PyObject*>
      {
            PyObject * operator() (int val) const
            {
               return ::PyInt_FromLong(val);
            }
            
            PyObject * operator() (double val) const
            {
               return ::PyFloat_FromDouble(val);
            }
            
            PyObject * operator() (UnicodeString const& s) const
            {
                std::string buffer;
                mapnik::to_utf8(s,buffer);
                PyObject *obj = Py_None;
                obj = ::PyUnicode_DecodeUTF8(buffer.c_str(),implicit_cast<ssize_t>(buffer.length()),0);                
                return obj;
            }
            
            PyObject * operator() (mapnik::value_null const& s) const
            {
               return NULL;
            }
      };
      
      struct mapnik_value_to_python
      {
            static PyObject* convert(mapnik::value const& v)
            {
               return boost::apply_visitor(value_converter(),v.base());
            }
      };
      
      // Forward declaration
      template <class Container, bool NoProxy, class DerivedPolicies>
      class map_indexing_suite2;

      namespace detail
      {
         template <class Container, bool NoProxy>
         class final_map_derived_policies
            : public map_indexing_suite2<Container,
                                         NoProxy, final_map_derived_policies<Container, NoProxy> > {};
      }
    
      template <
         class Container,
         bool NoProxy = false,
         class DerivedPolicies
         = detail::final_map_derived_policies<Container, NoProxy> >
      class map_indexing_suite2
         : public indexing_suite<
         Container
         , DerivedPolicies
         , NoProxy
         , true
         , typename Container::value_type::second_type
         , typename Container::key_type
         , typename Container::key_type
         >
      {
         public:

            typedef typename Container::value_type value_type;
            typedef typename Container::value_type::second_type data_type;
            typedef typename Container::key_type key_type;
            typedef typename Container::key_type index_type;
            typedef typename Container::size_type size_type;
            typedef typename Container::difference_type difference_type;

            template <class Class>
            static void
            extension_def(Class& cl)
            {
               
            }

            static data_type&
            get_item(Container& container, index_type i_)
            {
               typename Container::iterator i = container.find(i_);
               if (i == container.end())
               {
                  PyErr_SetString(PyExc_KeyError, "Invalid key");
                  throw_error_already_set();
               }
               return i->second;
            }
            
            static void
            set_item(Container& container, index_type i, data_type const& v)
            {
               container[i] = v;
            }
            
            static void
            delete_item(Container& container, index_type i)
            {
               container.erase(i);
            }
            
            static size_t
            size(Container& container)
            {
               return container.size();
            }
            
            static bool
            contains(Container& container, key_type const& key)
            {
               return container.find(key) != container.end();
            }
            
            static bool
            compare_index(Container& container, index_type a, index_type b)
            {
               return container.key_comp()(a, b);
            }
            
            static index_type
            convert_index(Container& /*container*/, PyObject* i_)
            {
               extract<key_type const&> i(i_);
               if (i.check())
               {
                  return i();
               }
               else
               {
                  extract<key_type> i(i_);
                  if (i.check())
                     return i();
               }
               
               PyErr_SetString(PyExc_TypeError, "Invalid index type");
               throw_error_already_set();
               return index_type();
            }
      };
      

      template <typename T1, typename T2>
      struct std_pair_to_tuple
      {
            static PyObject* convert(std::pair<T1, T2> const& p)
            {
               return boost::python::incref(
                  boost::python::make_tuple(p.first, p.second).ptr());
            }
      };
      
      template <typename T1, typename T2>
      struct std_pair_to_python_converter
      {
            std_pair_to_python_converter()
            {
               boost::python::to_python_converter<
               std::pair<T1, T2>,
                  std_pair_to_tuple<T1, T2> >();
            }
      };
   }
}

void export_feature()
{
   using namespace boost::python;
   using mapnik::Feature;
   
   std_pair_to_python_converter<std::string const,mapnik::value>();
   to_python_converter<mapnik::value,mapnik_value_to_python>();
   class_<Feature,boost::shared_ptr<Feature>,
      boost::noncopyable>("Feature",no_init)
      .def("id",&Feature::id)
      .def("__str__",&Feature::to_string)
      .add_property("properties", 
                    make_function(&Feature::props,return_value_policy<reference_existing_object>()))
//      .def("add_geometry", // TODO define more mapnik::Feature methods
      .def("num_geometries",&Feature::num_geometries)
      .def("get_geometry", make_function(get_geom1,return_value_policy<reference_existing_object>()))
      .def("envelope", &Feature::envelope)
      ;

   class_<std::map<std::string, mapnik::value> >("Properties")
      .def(map_indexing_suite2<std::map<std::string, mapnik::value>, true >())
      .def("iteritems",iterator<std::map<std::string,mapnik::value> > ())
      ;
}
