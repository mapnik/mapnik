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

// boost
#include <boost/python/suite/indexing/indexing_suite.hpp>
//#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/iterator.hpp>
#include <boost/python/call_method.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/python.hpp>

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/feature_kv_iterator.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/json/geojson_generator.hpp>

namespace {

using mapnik::Feature;
using mapnik::geometry_utils;
using mapnik::from_wkt;
using mapnik::context_type;
using mapnik::context_ptr;
using mapnik::feature_kv_iterator;

mapnik::geometry_type const& (mapnik::Feature::*get_geometry_by_const_ref)(unsigned) const = &mapnik::Feature::get_geometry;
boost::ptr_vector<mapnik::geometry_type> const& (mapnik::Feature::*get_paths_by_const_ref)() const = &mapnik::Feature::paths;

void feature_add_geometries_from_wkb(Feature &feature, std::string wkb)
{
    geometry_utils::from_wkb(feature.paths(), wkb.c_str(), wkb.size());
}

void feature_add_geometries_from_wkt(Feature &feature, std::string wkt)
{
    bool result = mapnik::from_wkt(wkt, feature.paths());
    if (!result) throw std::runtime_error("Failed to parse WKT");
}

std::string feature_to_geojson(Feature const& feature)
{
    std::string json;
    mapnik::json::feature_generator g;
    if (!g.generate(json,feature))
    {
        throw std::runtime_error("Failed to generate GeoJSON");
    }
    return json;
}

mapnik::value  __getitem__(Feature const& feature, std::string const& name)
{
    return feature.get(name);
}

mapnik::value  __getitem2__(Feature const& feature, std::size_t index)
{
    return feature.get(index);
}

void __setitem__(Feature & feature, std::string const& name, mapnik::value const& val)
{
    feature.put_new(name,val);
}

boost::python::dict attributes(Feature const& f)
{
    boost::python::dict attributes;
    feature_kv_iterator itr = f.begin();
    feature_kv_iterator end = f.end();

    for ( ;itr!=end; ++itr)
    {
        attributes[boost::get<0>(*itr)] = boost::get<1>(*itr);
    }

    return attributes;
}

} // end anonymous namespace

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

    // Python to mapnik::value converters
    implicitly_convertible<int,mapnik::value>();
    implicitly_convertible<double,mapnik::value>();
    implicitly_convertible<UnicodeString,mapnik::value>();
    implicitly_convertible<bool,mapnik::value>();

    UnicodeString_from_python_str();

    class_<context_type,context_ptr,boost::noncopyable>
        ("Context",init<>("Default ctor."))
        .def("push", &context_type::push)
        ;

    class_<Feature,boost::shared_ptr<Feature>,
        boost::noncopyable>("Feature",init<context_ptr,int>("Default ctor."))
        .def("id",&Feature::id)
        .def("__str__",&Feature::to_string)
        .def("add_geometries_from_wkb", &feature_add_geometries_from_wkb)
        .def("add_geometries_from_wkt", &feature_add_geometries_from_wkt)
        .def("add_geometry", &Feature::add_geometry)
        .def("num_geometries",&Feature::num_geometries)
        .def("get_geometry", make_function(get_geometry_by_const_ref,return_value_policy<reference_existing_object>()))
        .def("geometries",make_function(get_paths_by_const_ref,return_value_policy<reference_existing_object>()))
        .def("envelope", &Feature::envelope)
        .def("has_key", &Feature::has_key)
        .add_property("attributes",&attributes)
        .def("__setitem__",&__setitem__)
        .def("__getitem__",&__getitem__)
        .def("__getitem__",&__getitem2__)
        .def("__len__", &Feature::size)
        .def("context",&Feature::context)
        .def("to_geojson",&feature_to_geojson)
        ;
}
