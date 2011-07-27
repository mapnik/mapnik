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
#include <mapnik/datasource.hpp>
#include <mapnik/wkb.hpp>
#include "mapnik_value_converter.hpp"

mapnik::geometry_type & (mapnik::Feature::*get_geom1)(unsigned) = &mapnik::Feature::get_geometry;

namespace {

using mapnik::Feature;
using mapnik::geometry_utils;

/*
void feature_add_wkb_geometry(Feature &feature, std::string wkb)
{
    geometry_utils::from_wkb(feature, wkb.c_str(), wkb.size(), true);
}
*/

void add_geometry(Feature & feature, std::auto_ptr<mapnik::geometry_type> geom)
{
    feature.add_geometry(geom.get());
    geom.release();
}

} // end anonymous namespace

namespace boost { namespace python {

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
    
    template <class Container,bool NoProxy = false,
              class DerivedPolicies = detail::final_map_derived_policies<Container, NoProxy> >
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
        extension_def(Class& /*cl*/)
        {
               
        }

        static data_type&
        get_item(Container& container, index_type i_)
        {
            typename Container::iterator i = container.props().find(i_);
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
            container.props().erase(i);
        }
          
        static size_t
        size(Container& container)
        {
            return container.props().size();
        }
          
        static bool
        contains(Container& container, key_type const& key)
        {
            return container.props().find(key) != container.end();
        }
            
        static bool
        compare_index(Container& container, index_type a, index_type b)
        {
            return container.props().key_comp()(a, b);
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

    }}

struct UnicodeString_from_python_str
{
    UnicodeString_from_python_str()
    {
        boost::python::converter::registry::push_back(
            &convertible,
            &construct,
            boost::python::type_id<UnicodeString>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (!(
#if PY_VERSION_HEX >= 0x03000000
                PyBytes_Check(obj_ptr) 
#else
                PyString_Check(obj_ptr) 
#endif
                || PyUnicode_Check(obj_ptr)))
            return 0;
        return obj_ptr;
    }

    static void construct(
        PyObject* obj_ptr,
        boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        char * value=0;
        if (PyUnicode_Check(obj_ptr)) {
            PyObject *encoded = PyUnicode_AsEncodedString(obj_ptr, "utf8", "replace");
            if (encoded) {
#if PY_VERSION_HEX >= 0x03000000
                value = PyBytes_AsString(encoded);
#else
                value = PyString_AsString(encoded);
#endif
                Py_DecRef(encoded);
            }
        } else {
#if PY_VERSION_HEX >= 0x03000000
            value = PyBytes_AsString(obj_ptr);
#else
            value = PyString_AsString(obj_ptr);
#endif
        }
        if (value == 0) boost::python::throw_error_already_set();
        void* storage = (
            (boost::python::converter::rvalue_from_python_storage<UnicodeString>*)
            data)->storage.bytes;
        new (storage) UnicodeString(value);
        data->convertible = storage;
    }
};

void export_feature()
{
    using namespace boost::python;
    using mapnik::Feature;
      
    implicitly_convertible<int,mapnik::value>();
    implicitly_convertible<double,mapnik::value>();
    implicitly_convertible<UnicodeString,mapnik::value>();
    implicitly_convertible<bool,mapnik::value>();

    std_pair_to_python_converter<std::string const,mapnik::value>();
    UnicodeString_from_python_str();
   
    class_<Feature,boost::shared_ptr<Feature>,
        boost::noncopyable>("Feature",init<int>("Default ctor."))
        .def("id",&Feature::id)
        .def("__str__",&Feature::to_string)
//        .def("add_geometry", &feature_add_wkb_geometry)
        .def("add_geometry", add_geometry)
        .def("num_geometries",&Feature::num_geometries)
        .def("get_geometry", make_function(get_geom1,return_value_policy<reference_existing_object>()))
        .def("envelope", &Feature::envelope)
        .def(map_indexing_suite2<Feature, true >())
        .def("iteritems",iterator<Feature> ())
        // TODO define more mapnik::Feature methods
        ;
}
