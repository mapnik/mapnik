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
#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>

// stl
#include <sstream>
#include <vector>

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/query.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/memory_datasource.hpp>


using mapnik::datasource;
using mapnik::memory_datasource;
using mapnik::layer_descriptor;
using mapnik::attribute_descriptor;

namespace
{
//user-friendly wrapper that uses Python dictionary
using namespace boost::python;
boost::shared_ptr<mapnik::datasource> create_datasource(const dict& d)
{
    bool bind=true;
    mapnik::parameters params;
    boost::python::list keys=d.keys();
    for (int i=0; i<len(keys); ++i)
    {
        std::string key = extract<std::string>(keys[i]);
        object obj = d[key];

        if (key == "bind")
        {
            bind = extract<bool>(obj)();
            continue;
        }

        extract<std::string> ex0(obj);
        extract<int> ex1(obj);
        extract<double> ex2(obj);

        if (ex0.check())
        {
            params[key] = ex0();
        }
        else if (ex1.check())
        {
            params[key] = ex1();
        }
        else if (ex2.check())
        {
            params[key] = ex2();
        }
    }

    return mapnik::datasource_cache::instance().create(params, bind);
}

boost::python::dict describe(boost::shared_ptr<mapnik::datasource> const& ds)
{
    boost::python::dict description;
    mapnik::layer_descriptor ld = ds->get_descriptor();
    description["type"] = ds->type();
    description["name"] = ld.get_name();
    description["geometry_type"] = ds->get_geometry_type();
    description["encoding"] = ld.get_encoding();
    return description;
}

boost::python::list fields(boost::shared_ptr<mapnik::datasource> const& ds)
{
    boost::python::list flds;
    if (ds)
    {
        layer_descriptor ld = ds->get_descriptor();
        std::vector<attribute_descriptor> const& desc_ar = ld.get_descriptors();
        std::vector<attribute_descriptor>::const_iterator it = desc_ar.begin();
        std::vector<attribute_descriptor>::const_iterator end = desc_ar.end();
        for (; it != end; ++it)
        {
            flds.append(it->get_name());
        }
    }
    return flds;
}
boost::python::list field_types(boost::shared_ptr<mapnik::datasource> const& ds)
{
    boost::python::list fld_types;
    if (ds)
    {
        layer_descriptor ld = ds->get_descriptor();
        std::vector<attribute_descriptor> const& desc_ar = ld.get_descriptors();
        std::vector<attribute_descriptor>::const_iterator it = desc_ar.begin();
        std::vector<attribute_descriptor>::const_iterator end = desc_ar.end();
        for (; it != end; ++it)
        {
            unsigned type = it->get_type();
            if (type == mapnik::Integer)
                // this crashes, so send back strings instead
                //fld_types.append(boost::python::object(boost::python::handle<>(&PyInt_Type)));
                fld_types.append(boost::python::str("int"));
            else if (type == mapnik::Float)
                fld_types.append(boost::python::str("float"));
            else if (type == mapnik::Double)
                fld_types.append(boost::python::str("float"));
            else if (type == mapnik::String)
                fld_types.append(boost::python::str("str"));
            else if (type == mapnik::Boolean)
                fld_types.append(boost::python::str("bool"));
            else if (type == mapnik::Geometry)
                fld_types.append(boost::python::str("geometry"));
            else if (type == mapnik::Object)
                fld_types.append(boost::python::str("object"));
            else
                fld_types.append(boost::python::str("unknown"));
        }
    }
    return fld_types;
}}

void export_datasource()
{
    using namespace boost::python;

    enum_<mapnik::datasource::datasource_t>("DataType")
        .value("Vector",mapnik::datasource::Vector)
        .value("Raster",mapnik::datasource::Raster)
        ;

    enum_<mapnik::datasource::geometry_t>("DataGeometryType")
        .value("Point",mapnik::datasource::Point)
        .value("LineString",mapnik::datasource::LineString)
        .value("Polygon",mapnik::datasource::Polygon)
        .value("Collection",mapnik::datasource::Collection)
        ;

    class_<datasource,boost::shared_ptr<datasource>,
        boost::noncopyable>("Datasource",no_init)
        .def("type",&datasource::type)
        .def("geometry_type",&datasource::get_geometry_type)
        .def("describe",&describe)
        .def("envelope",&datasource::envelope)
        .def("features",&datasource::features)
        .def("bind",&datasource::bind)
        .def("fields",&fields)
        .def("field_types",&field_types)
        .def("features_at_point",&datasource::features_at_point, (arg("coord"),arg("tolerance")=0))
        .def("params",&datasource::params,return_value_policy<copy_const_reference>(),
             "The configuration parameters of the data source. "
             "These vary depending on the type of data source.")
        ;

    def("CreateDatasource",&create_datasource);

    class_<memory_datasource, bases<datasource>, boost::noncopyable>("MemoryDatasource", init<>())
        .def("add_feature",&memory_datasource::push,
             "Adds a Feature:\n"
             ">>> ms = MemoryDatasource()\n"
             ">>> feature = Feature(1)\n"
             ">>> ms.add_feature(Feature(1))\n")
        .def("num_features",&memory_datasource::size)
        ;
}
